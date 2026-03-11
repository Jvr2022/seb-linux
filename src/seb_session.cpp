#include "seb_session.h"

#include "browser/request_interceptor.h"
#include "browser_window.h"

#include <QApplication>
#include <QAuthenticator>
#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLocale>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QUrl>
#include <QWebEngineCookieStore>
#include <QWebEngineDownloadRequest>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

SebSession::SebSession(const seb::SebSettings &settings, ResourceOpener opener, QObject *parent)
    : QObject(parent)
    , settings_(settings)
    , opener_(std::move(opener))
{
    const bool useTemporaryProfile =
        settings_.browser.deleteCacheOnShutdown ||
        settings_.browser.deleteCookiesOnShutdown ||
        settings_.browser.deleteCookiesOnStartup;

    profile_.reset(new QWebEngineProfile(this));

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

    profile_->settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    profile_->settings()->setAttribute(QWebEngineSettings::PdfViewerEnabled, settings_.browser.allowPdfReader);
    profile_->settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, false);
    profile_->settings()->setAttribute(QWebEngineSettings::HyperlinkAuditingEnabled, false);
    profile_->settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, true);
    profile_->setSpellCheckEnabled(settings_.browser.allowSpellChecking);
    profile_->setSpellCheckLanguages(QStringList{QLocale::system().bcp47Name()});
    profile_->setHttpUserAgent(buildUserAgent());

    interceptor_.reset(new seb::browser::RequestInterceptor(settings_, this));
    profile_->setUrlRequestInterceptor(interceptor_.data());

    connect(profile_.data(), &QWebEngineProfile::downloadRequested, this, &SebSession::handleDownloadRequested);

    if (settings_.browser.deleteCookiesOnStartup) {
        profile_->cookieStore()->deleteAllCookies();
    }
}

SebSession::~SebSession() = default;

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

QWebEngineProfile *SebSession::profile() const
{
    return profile_.data();
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

void SebSession::handleDownloadRequested(QWebEngineDownloadRequest *download)
{
    if (!download) {
        return;
    }

    const QString fileName = download->downloadFileName().isEmpty()
        ? QStringLiteral("download")
        : download->downloadFileName();
    const bool sebConfig = fileName.endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive);

    if (!settings_.browser.allowDownloads && !(sebConfig && settings_.browser.allowConfigurationDownloads)) {
        download->cancel();
        return;
    }

    if (sebConfig && !settings_.browser.allowConfigurationDownloads) {
        download->cancel();
        return;
    }

    if (sebConfig) {
        const QUrl url = download->url();
        download->cancel();
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
            download->cancel();
            return;
        }

        destination = selected;
    }

    const QFileInfo info(destination);
    download->setDownloadDirectory(info.dir().path());
    download->setDownloadFileName(info.fileName());
    download->accept();
}

QString SebSession::buildUserAgent() const
{
    QString agent = profile_->httpUserAgent();

    if (settings_.browser.useCustomUserAgent && !settings_.browser.customUserAgent.isEmpty()) {
        agent = settings_.browser.customUserAgent.trimmed();
    }

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
