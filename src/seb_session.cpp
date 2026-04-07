#include "seb_session.h"
#include <cstdlib>
#include "browser/engines/engine_factory.h"
#include "browser/request_interceptor.h"
#include "applications/application_manager.h"
#include "browser_window.h"

#include <QApplication>
#include <QAuthenticator>
#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTimer>
#include <QUrl>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLocale>
#include <QRegularExpression>

#include "browser/contracts/i_engine_provider.h"
#include "browser/contracts/i_webprofile.h"

namespace {

QString getCachedSebVersion()
{
    static QString cachedVersion;
    if (!cachedVersion.isEmpty()) {
        return cachedVersion;
    }

    cachedVersion = QStringLiteral("3.10.1");

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("https://api.github.com/repos/SafeExamBrowser/seb-win-refactoring/releases/latest")));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SEB Linux Qt/") + QApplication::applicationVersion());

    QEventLoop loop;
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();

    if (!reply->isFinished()) {
        reply->abort();
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QByteArray responseData = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseData);
        if (json.isObject()) {
            QString tag = json.object().value(QStringLiteral("tag_name")).toString();
            if (!tag.isEmpty()) {
                if (tag.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
                    tag = tag.mid(1);
                }
                cachedVersion = tag;
            }
        }
    }
    reply->deleteLater();

    return cachedVersion;
}

}  // namespace

SebSession::SebSession(const seb::SebSettings &settings, ResourceOpener opener, QObject *parent)
    : QObject(parent)
    , settings_(settings)
    , opener_(std::move(opener))
{
    const bool useTemporaryProfile =
        settings_.browser.deleteCacheOnShutdown ||
        settings_.browser.deleteCookiesOnShutdown ||
        settings_.browser.deleteCookiesOnStartup;

    engineProvider_ = seb::browser::createEngineProvider();
    if (!engineProvider_) {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setWindowTitle(tr("Unsupported Device"));
        msgBox.setText(tr("Safe Exam Browser is not supported on your device."));
        msgBox.setInformativeText(tr("If you want support please open a GitHub issue with details about your system configuration."));
        QPushButton *issueButton = msgBox.addButton(tr("Open GitHub Issue"), QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Close);
        msgBox.exec();

        if (msgBox.clickedButton() == issueButton) {
            QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/Jvr2022/seb-linux/issues")));
        }
        std::exit(1);
    }
    profile_ = engineProvider_->createProfile(this);

    if (useTemporaryProfile) {
        profileDirectory_ = std::make_unique<QTemporaryDir>(
            QDir::tempPath() + QStringLiteral("/seb-linux-profile-XXXXXX"));
        profile_->setPersistentStoragePath(profileDirectory_->path());
        profile_->setCachePath(profileDirectory_->path() + QStringLiteral("/cache"));
    } else {
        const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        profile_->setPersistentStoragePath(baseDir + QStringLiteral("/profile"));
        profile_->setCachePath(baseDir + QStringLiteral("/cache"));
    }

    if (settings_.browser.useTemporaryDownAndUploadDirectory) {
        downloadDirectory_ = std::make_unique<QTemporaryDir>(
            QDir::tempPath() + QStringLiteral("/seb-linux-downloads-XXXXXX"));
        profile_->setDownloadPath(downloadDirectory_->path());
    } else {
        profile_->setDownloadPath(defaultDownloadDirectory());
    }

    profile_->setPdfViewerEnabled(settings_.browser.allowPdfReader);
    profile_->setSpellCheckEnabled(settings_.browser.allowSpellChecking);
    profile_->setSpellCheckLanguages(QStringList{QLocale::system().bcp47Name()});
    profile_->setHttpUserAgent(buildUserAgent());
    profile_->setDevBypass(settings_.devBypass);

    interceptor_.reset(new seb::browser::RequestInterceptor(settings_));
    profile_->setUrlRequestInterceptor(interceptor_.get());

    connect(profile_.get(), &seb::browser::contracts::IWebProfile::downloadRequested,
            this, &SebSession::handleDownloadRequested);

    if (settings_.browser.deleteCookiesOnStartup) {
        profile_->deleteAllCookies();
    }

    applicationManager_ = std::make_unique<seb::applications::ApplicationManager>(settings_.applications, this);
    connect(applicationManager_.get(), &seb::applications::ApplicationManager::applicationsChanged, this, &SebSession::externalApplicationsChanged);
    applicationManager_->initialize();
    applicationManager_->autoStart();
}

SebSession::~SebSession()
{
    if (applicationManager_) {
        applicationManager_->terminate();
    }
}

BrowserWindow *SebSession::createWindow(const QUrl &url, bool isMainWindow)
{
    const auto &windowSettings = isMainWindow ? settings_.browser.mainWindow : settings_.browser.additionalWindow;
    auto *window = new BrowserWindow(*this, windowSettings, url, isMainWindow);
    if (!isMainWindow) {
        window->setAttribute(Qt::WA_DeleteOnClose);
    }
    return window;
}

bool SebSession::applyProxyAuthentication(const QString &proxyHost, QAuthenticator *authenticator) const
{
    if (!authenticator) {
        return false;
    }

    for (const seb::ProxyConfiguration &proxy : settings_.browser.proxy.proxies) {
        if (proxy.requiresAuthentication &&
            proxy.host.compare(proxyHost, Qt::CaseInsensitive) == 0) {
            authenticator->setUser(proxy.username);
            authenticator->setPassword(proxy.password);
            return true;
        }
    }

    return false;
}

bool SebSession::confirmReload(QWidget *parent) const
{
    return QMessageBox::question(
               parent,
               QStringLiteral("Reload Page"),
               QStringLiteral("Reloading may reset the current exam page state. Continue?"),
               QMessageBox::Yes | QMessageBox::No,
               QMessageBox::No) == QMessageBox::Yes;
}

bool SebSession::confirmQuitUrl(QWidget *parent) const
{
    return QMessageBox::question(
               parent,
               QStringLiteral("Quit Session"),
               QStringLiteral("The exam page requested that Safe Exam Browser should quit. Continue?"),
               QMessageBox::Yes | QMessageBox::No,
               QMessageBox::No) == QMessageBox::Yes;
}

bool SebSession::isPopupAllowed(const QUrl &opener, const QUrl &target, bool *openInSameWindow) const
{
    if (openInSameWindow) {
        *openInSameWindow = false;
    }

    const bool sameHost = opener.host().compare(target.host(), Qt::CaseInsensitive) == 0;
    switch (settings_.browser.popupPolicy) {
    case seb::PopupPolicy::Allow:
        return true;
    case seb::PopupPolicy::AllowSameHost:
        return sameHost;
    case seb::PopupPolicy::AllowSameHostAndWindow:
        if (openInSameWindow) {
            *openInSameWindow = true;
        }
        return sameHost;
    case seb::PopupPolicy::AllowSameWindow:
        if (openInSameWindow) {
            *openInSameWindow = true;
        }
        return true;
    case seb::PopupPolicy::Block:
        return false;
    }

    return false;
}

bool SebSession::isQuitUrl(const QUrl &url) const
{
    if (settings_.browser.quitUrl.isEmpty()) {
        return false;
    }
    return normalizeUrl(url) == normalizeUrl(QUrl(settings_.browser.quitUrl));
}

bool SebSession::promptForHomeNavigation(QWidget *parent) const
{
    if (!settings_.browser.homeNavigationMessage.isEmpty()) {
        const auto answer = QMessageBox::question(
            parent,
            QStringLiteral("Navigate Home"),
            settings_.browser.homeNavigationMessage,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return false;
        }
    }

    if (settings_.browser.homeNavigationRequiresPassword &&
        !settings_.security.quitPasswordHash.isEmpty()) {
        return promptForPassword(
            parent,
            QStringLiteral("Quit Password Required"),
            QStringLiteral("Enter the quit password to navigate home."));
    }

    return true;
}

bool SebSession::requestApplicationQuit(QWidget *parent, const QString &reason) const
{
    if (!settings_.security.allowTermination) {
        QMessageBox::warning(
            parent,
            QStringLiteral("Termination Disabled"),
            QStringLiteral("Termination is disabled by the active configuration."));
        return false;
    }

    if (!settings_.security.quitPasswordHash.isEmpty()) {
        return promptForPassword(parent, QStringLiteral("Quit Password Required"), reason);
    }

    return true;
}

const seb::SebSettings &SebSession::settings() const
{
    return settings_;
}

seb::browser::contracts::IWebProfile *SebSession::profile() const
{
    return profile_.get();
}

seb::browser::contracts::IEngineProvider *SebSession::engineProvider() const
{
    return engineProvider_.get();
}

QUrl SebSession::homeUrl() const
{
    return QUrl(settings_.browser.homeUrl);
}

QUrl SebSession::initialUrl() const
{
    QUrl url(settings_.browser.startUrl);
    if (!url.isValid()) {
        return url;
    }

    if (settings_.browser.useQueryParameter && !settings_.browser.startUrlQuery.isEmpty()) {
        const QString query = settings_.browser.startUrlQuery.startsWith('?')
            ? settings_.browser.startUrlQuery.mid(1)
            : settings_.browser.startUrlQuery;

        QString existing = url.query(QUrl::FullyDecoded);
        if (existing.isEmpty()) {
            url.setQuery(query);
        } else {
            existing += QLatin1Char('&') + query;
            url.setQuery(existing);
        }
    }

    return url;
}

bool SebSession::openSebResource(const QUrl &url, QWidget *parent) const
{
    if (!opener_) {
        return false;
    }

    const QString resource = url.isLocalFile() ? url.toLocalFile() : url.toString();
    return opener_(resource, parent);
}

QList<BrowserWindow *> SebSession::browserWindows() const
{
    return browserWindows_;
}

QList<seb::applications::ExternalApplication *> SebSession::externalApplications() const
{
    return applicationManager_ ? applicationManager_->applications() : QList<seb::applications::ExternalApplication *>{};
}

void SebSession::registerBrowserWindow(BrowserWindow *window)
{
    if (!window || browserWindows_.contains(window)) {
        return;
    }

    browserWindows_.push_back(window);
    emit browserWindowsChanged();
}

void SebSession::unregisterBrowserWindow(BrowserWindow *window)
{
    if (!window) {
        return;
    }

    browserWindows_.removeAll(window);
    emit browserWindowsChanged();
}

void SebSession::notifyBrowserWindowStateChanged(BrowserWindow *window)
{
    if (window && browserWindows_.contains(window)) {
        emit browserWindowsChanged();
    }
}

void SebSession::activateWindow(BrowserWindow *window)
{
    if (!window) {
        return;
    }

    window->show();
    if (window->isMinimized()) {
        window->showNormal();
    }
    window->raise();
    window->activateWindow();
}

void SebSession::handleDownloadRequested(const QUrl &url, const QString &suggestedFilename, bool &accepted, QString &downloadDirectory)
{
    const QString fileName = suggestedFilename.isEmpty()
        ? QStringLiteral("download")
        : suggestedFilename;
    const bool sebConfig = fileName.endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive);

    if (!settings_.browser.allowDownloads && !(sebConfig && settings_.browser.allowConfigurationDownloads)) {
        accepted = false;
        return;
    }

    if (sebConfig && !settings_.browser.allowConfigurationDownloads) {
        accepted = false;
        return;
    }

    if (sebConfig) {
        accepted = false;
        openSebResource(url, QApplication::activeWindow());
        return;
    }

    QString destination = QDir(defaultDownloadDirectory()).filePath(fileName);
    if (settings_.browser.allowCustomDownAndUploadLocation) {
        const QString selected = QFileDialog::getSaveFileName(
            QApplication::activeWindow(),
            QStringLiteral("Save Download"),
            destination);

        if (selected.isEmpty()) {
            accepted = false;
            return;
        }

        destination = selected;
    }

    const QFileInfo info(destination);
    downloadDirectory = info.dir().path();
    accepted = true;
}

QString SebSession::buildUserAgent() const
{
    QString agent;

    if (settings_.browser.useCustomUserAgent && !settings_.browser.customUserAgent.isEmpty()) {
        agent = settings_.browser.customUserAgent.trimmed();
    } else {
        const QString chromeVersion = QStringLiteral("110.0.0.0");
        agent = QStringLiteral("Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/") + chromeVersion;
    }

    const QString sebVersion = QStringLiteral("SEB/") + getCachedSebVersion();
    if (!agent.endsWith(' ')) {
        agent += QLatin1Char(' ');
    }
    agent += sebVersion;

    if (!settings_.browser.userAgentSuffix.trimmed().isEmpty()) {
        if (!agent.endsWith(' ')) {
            agent += QLatin1Char(' ');
        }
        agent += settings_.browser.userAgentSuffix.trimmed();
    }

    return agent;
}

QString SebSession::defaultDownloadDirectory() const
{
    if (downloadDirectory_ && downloadDirectory_->isValid()) {
        return downloadDirectory_->path();
    }

    if (!settings_.browser.downloadDirectory.trimmed().isEmpty()) {
        return settings_.browser.downloadDirectory.trimmed();
    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (path.isEmpty()) {
        path = QDir::homePath();
    }
    return path;
}

QString SebSession::normalizeUrl(const QUrl &url) const
{
    QString normalized = url.adjusted(QUrl::NormalizePathSegments | QUrl::RemoveFragment).toString();
    while (normalized.endsWith('/') && normalized.size() > 1) {
        normalized.chop(1);
    }
    return normalized;
}

bool SebSession::promptForPassword(QWidget *parent, const QString &title, const QString &message) const
{
    bool accepted = false;
    const QString password = QInputDialog::getText(
        parent,
        title,
        message,
        QLineEdit::Password,
        QString(),
        &accepted);

    if (!accepted) {
        return false;
    }

    const QByteArray digest = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    const QString hash = QString::fromLatin1(digest.toHex());
    if (hash.compare(settings_.security.quitPasswordHash, Qt::CaseInsensitive) == 0) {
        return true;
    }

    QMessageBox::warning(parent, title, QStringLiteral("The entered quit password is invalid."));
    return false;
}
