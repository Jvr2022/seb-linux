#include "seb_session.h"
#include <cstdlib>
#include "browser/engines/engine_factory.h"
#include "browser/request_interceptor.h"
#include "configuration/cryptography/key_generator.h"
#include "applications/application_manager.h"
#include "browser_window.h"
#include "server/server_proxy.h"

#include <QApplication>
#include <QAuthenticator>
#include <QCryptographicHash>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QDesktopServices>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVBoxLayout>
#include <QDebug>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QUrl>
#include <QStandardPaths>
#include <QLocale>

#include "browser/contracts/i_engine_provider.h"
#include "browser/contracts/i_webprofile.h"
#include "browser/configuration_resource_detector.h"

namespace {

void appendUserAgentPart(QString &agent, const QString &part)
{
    const QString value = part.trimmed();
    if (value.isEmpty()) {
        return;
    }

    if (!agent.isEmpty()) {
        agent += QLatin1Char(' ');
    }
    agent += value;
}

}  // namespace

SebSession::SebSession(
    const seb::configuration::contracts::SessionConfiguration &configuration,
    ResourceOpener opener,
    QObject *parent,
    seb::browser::contracts::IEngineProvider *sharedEngineProvider,
    seb::browser::contracts::IWebProfile *sharedProfile,
    std::unique_ptr<seb::server::ServerProxy> serverProxy)
    : QObject(parent)
    , configuration_(configuration)
    , opener_(std::move(opener))
    , serverProxy_(std::move(serverProxy))
{
    if (serverProxy_) {
        connect(profilePtr_, &seb::browser::contracts::IWebProfile::cookieAdded,
                this, &SebSession::handleServerCookie);
        serverProxy_->setParent(this);
    }
    const auto &settings = configuration_.settings;
    const bool useTemporaryProfile = false;

    if (sharedEngineProvider && sharedProfile) {
        engineProviderPtr_ = sharedEngineProvider;
        profilePtr_ = sharedProfile;
    } else {
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

        if (useTemporaryProfile) {
            profileDirectory_ = std::make_unique<QTemporaryDir>(
                QDir::tempPath() + QStringLiteral("/seb-linux-profile-XXXXXX"));
        }

        const QString storageName = QStringLiteral("seb-session-") +
            QUuid::createUuid().toString(QUuid::WithoutBraces);
        profile_ = engineProvider_->createProfile(this, storageName);
        engineProviderPtr_ = engineProvider_.get();
        profilePtr_ = profile_.get();
    }

    if (!profilePtr_) {
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

    if (!serverProxy_ && settings.SessionMode == seb::settings::SessionMode::Server &&
        !configuration_.appConfig.serverApi.isEmpty()) {
        auto proxy = std::make_unique<seb::server::ServerProxy>(configuration_.appConfig, this);
        QString ignored;
        if (proxy->initializeConnection(
                configuration_.appConfig.serverApi,
                configuration_.appConfig.serverConnectionToken,
                configuration_.appConfig.serverExamId,
                configuration_.appConfig.serverOauth2Token,
                settings.Server,
                &ignored)) {
            serverProxy_ = std::move(proxy);
        }
    }

    if (serverProxy_) {
        connect(serverProxy_.get(), &seb::server::ServerProxy::terminationRequested,
                this, [] { qDebug() << "SEB Server requested termination"; QCoreApplication::quit(); });
        connect(serverProxy_.get(), &seb::server::ServerProxy::handConfirmed,
                this, &SebSession::serverHandConfirmed);
        connect(serverProxy_.get(), &seb::server::ServerProxy::lockScreenRequested,
                this, [this](const QString &message) {
                    if (serverLockScreen_) {
                        if (auto *label = serverLockScreen_->findChild<QLabel *>()) {
                            label->setText(message);
                        }
                        serverLockScreen_->showFullScreen();
                        return;
                    }
                    auto *overlay = new QWidget();
                    overlay->setAttribute(Qt::WA_DeleteOnClose, false);
                    overlay->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                    overlay->setStyleSheet(QStringLiteral("background: #000000; color: #ffffff;"));
                    auto *layout = new QVBoxLayout(overlay);
                    auto *label = new QLabel(message, overlay);
                    label->setAlignment(Qt::AlignCenter);
                    label->setWordWrap(true);
                    layout->addWidget(label);
                    auto *confirm = new QPushButton(QStringLiteral("Confirm"), overlay);
                    confirm->setFixedWidth(160);
                    confirm->setProperty("serverLockConfirm", true);
                    layout->addWidget(confirm, 0, Qt::AlignHCenter);
                    connect(confirm, &QPushButton::clicked, this, [this] {
                        if (serverProxy_) {
                            serverProxy_->confirmLockScreen();
                        }
                    });
                    serverLockScreen_ = overlay;
                    overlay->showFullScreen();
                });
        connect(serverProxy_.get(), &seb::server::ServerProxy::lockScreenConfirmed,
                this, [this] {
                    if (serverLockScreen_) {
                        serverLockScreen_->close();
                        serverLockScreen_->deleteLater();
                        serverLockScreen_ = nullptr;
                    }
                });
        connect(serverProxy_.get(), &seb::server::ServerProxy::proctoringInstructionReceived,
                this, [](const QString &message) { qDebug() << "SEB Server proctoring instruction:" << message; });
        connect(serverProxy_.get(), &seb::server::ServerProxy::proctoringConfigurationReceived,
                this, [](bool allowChat, bool receiveAudio, bool receiveVideo) {
                    qDebug() << "SEB Server proctoring configuration:" << allowChat << receiveAudio << receiveVideo;
                });
    }

    connect(profilePtr_, &seb::browser::contracts::IWebProfile::downloadFinished,
            this, [this](const seb::browser::contracts::DownloadResult &result) {
                if (!reconfigurationDownloadId_ ||
                    result.id != *reconfigurationDownloadId_) {
                    return;
                }
                reconfigurationDownloadId_.reset();
                if (!result.success || result.localPath.isEmpty()) {
                    return;
                }

                const QString directory = configurationDownloadDirectory_
                    ? configurationDownloadDirectory_->path()
                    : QString();
                const QString cleanPath = QDir::cleanPath(result.localPath);
                const QString cleanDirectory = QDir::cleanPath(directory);
                const bool insideDirectory =
                    !cleanDirectory.isEmpty() &&
                    (cleanPath == cleanDirectory ||
                     cleanPath.startsWith(cleanDirectory + QDir::separator()));
                if (!insideDirectory ||
                    !result.localPath.endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive)) {
                    return;
                }
                openSebResource(QUrl::fromLocalFile(result.localPath), QApplication::activeWindow());
            });

    if (configuration_.reuseBrowserProfile && !configuration_.browserProfilePath.isEmpty()) {
        profilePtr_->setPersistentStoragePath(configuration_.browserProfilePath);
        profilePtr_->setCachePath(configuration_.browserProfilePath + QStringLiteral("/cache"));
    } else if (useTemporaryProfile && profileDirectory_) {
        profilePtr_->setPersistentStoragePath(profileDirectory_->path());
        profilePtr_->setCachePath(profileDirectory_->path() + QStringLiteral("/cache"));
    } else {
        const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        profilePtr_->setPersistentStoragePath(baseDir + QStringLiteral("/profile"));
        profilePtr_->setCachePath(baseDir + QStringLiteral("/cache"));
    }

    if (settings.Browser.UseTemporaryDownAndUploadDirectory) {
        downloadDirectory_ = std::make_unique<QTemporaryDir>(
            QDir::tempPath() + QStringLiteral("/seb-linux-downloads-XXXXXX"));
        profilePtr_->setDownloadPath(downloadDirectory_->path());
    } else {
        profilePtr_->setDownloadPath(defaultDownloadDirectory());
    }

    profilePtr_->setPdfViewerEnabled(settings.Browser.AllowPdfReader);
    profilePtr_->setSpellCheckEnabled(settings.Browser.AllowSpellChecking);
    const QString locale = QLocale::system().bcp47Name();
    profilePtr_->setSpellCheckLanguages(QStringList{locale});
    profilePtr_->setHttpAcceptLanguage(locale);
    profilePtr_->setPersistSessionCookies(
        !settings.Browser.DeleteCookiesOnStartup ||
        !settings.Browser.DeleteCookiesOnShutdown);
    const QString nativeUserAgent = profilePtr_->defaultHttpUserAgent();
    const QString userAgent = buildUserAgent();
    profilePtr_->setHttpUserAgent(userAgent);
    qDebug() << "Linux application version:" << configuration_.appConfig.programBuildVersion;
    qDebug() << "SEB compatibility version:" << configuration_.appConfig.sebCompatibilityVersion;
    qDebug() << "Browser engine User-Agent:" << nativeUserAgent;
    qDebug() << "Browser User-Agent:" << userAgent;
    profilePtr_->setDevBypass(false);

    keyGenerator_ = std::make_unique<seb::configuration::cryptography::KeyGenerator>(configuration_.appConfig);
    if (!settings.Browser.CustomBrowserExamKey.isEmpty()) {
        keyGenerator_->useCustomBrowserExamKey(settings.Browser.CustomBrowserExamKey);
    }

    interceptor_.reset(new seb::browser::RequestInterceptor(settings.Browser, *keyGenerator_));
    profilePtr_->setUrlRequestInterceptor(interceptor_.data());

    connect(profilePtr_, &seb::browser::contracts::IWebProfile::downloadRequested,
            this, &SebSession::handleDownloadRequested);

    if (settings.Browser.DeleteCookiesOnStartup) {
        profilePtr_->deleteAllCookies();
    }

    applicationManager_ = std::make_unique<seb::applications::ApplicationManager>(settings.Applications, this);
    connect(applicationManager_.get(), &seb::applications::ApplicationManager::applicationsChanged, this, &SebSession::externalApplicationsChanged);
    applicationManager_->initialize();
    applicationManager_->autoStart();
}

SebSession::~SebSession()
{
    if (serverLockScreen_) {
        serverLockScreen_->close();
        serverLockScreen_->deleteLater();
        serverLockScreen_ = nullptr;
    }
    if (profilePtr_ && !configuration_.isBrowserResource) {
        if (configuration_.settings.Browser.DeleteCookiesOnShutdown) {
            profilePtr_->deleteAllCookies();
        }
        if (configuration_.settings.Browser.DeleteCacheOnShutdown &&
            configuration_.settings.Browser.DeleteCookiesOnShutdown) {
            profilePtr_->clearHttpCache();
        }
    }
    if (applicationManager_) {
        applicationManager_->terminate();
    }
}

std::unique_ptr<seb::server::ServerProxy> SebSession::takeServerProxy()
{
    if (serverProxy_) {
        serverProxy_->setParent(nullptr);
    }
    return std::move(serverProxy_);
}

bool SebSession::isServerSession() const
{
    return configuration_.settings.SessionMode == seb::settings::SessionMode::Server;
}

bool SebSession::raiseServerHand(const QString &message)
{
    if (!serverProxy_ || !isServerSession() ||
        !configuration_.settings.Server.Invigilation.ShowRaiseHandNotification) {
        return false;
    }
    if (configuration_.settings.Server.Invigilation.ForceRaiseHandMessage && message.trimmed().isEmpty()) {
        return false;
    }
    return serverProxy_->raiseHand(message).success;
}

bool SebSession::lowerServerHand()
{
    return serverProxy_ && isServerSession() && serverProxy_->lowerHand().success;
}

void SebSession::handleServerCookie(const QNetworkCookie &cookie)
{
    if (!serverProxy_ || !isServerSession()) {
        return;
    }

    QString identifier;
    const QByteArray name = cookie.name();
    const QByteArray value = cookie.value();
    if (name == QByteArrayLiteral("SEB-LMS-USER-ID")) {
        identifier = QString::fromUtf8(value);
    } else if (name == QByteArrayLiteral("edx-user-info")) {
        QByteArray decoded = QByteArray::fromPercentEncoding(value);
        decoded.replace("\\\"", "\"");
        decoded.replace("\\054", ",");
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(decoded, &parseError);
        if (document.isObject()) {
            identifier = document.object().value(QStringLiteral("username")).toString();
        }
    } else if (name == QByteArrayLiteral("MoodleSession")) {
        const QString domain = cookie.domain().trimmed();
        if (!domain.isEmpty()) {
            const QString scheme = cookie.isSecure() ? QStringLiteral("https") : QStringLiteral("http");
            const QString path = QStringLiteral("/mod/quiz/accessrule/sebserver/classes/external/user.php");
            QNetworkRequest request(QUrl(scheme + QStringLiteral("://") + domain + path));
            request.setRawHeader(QByteArrayLiteral("Cookie"),
                                 name + QByteArrayLiteral("=") + value);
            auto *reply = serverIdentityNetwork_.get(request);
            connect(reply, &QNetworkReply::finished, this, [this, reply] {
                const QByteArray body = reply->readAll().trimmed();
                bool ok = false;
                const int id = body.toInt(&ok);
                if (ok && id > 0 && serverProxy_ && isServerSession()) {
                    const QString identifier = QString::number(id);
                    if (identifier != serverUserIdentifier_) {
                        serverUserIdentifier_ = identifier;
                        serverProxy_->sendUserIdentifier(identifier);
                    }
                }
                reply->deleteLater();
            });
        }
    }

    if (!identifier.isEmpty() && identifier != serverUserIdentifier_) {
        serverUserIdentifier_ = identifier;
        serverProxy_->sendUserIdentifier(identifier);
    }
}

BrowserWindow *SebSession::createWindow(const QUrl &url, bool isMainWindow)
{
    const auto &windowSettings = isMainWindow
        ? configuration_.settings.Browser.MainWindow
        : configuration_.settings.Browser.AdditionalWindow;
    auto *window = new BrowserWindow(*this, windowSettings, url, isMainWindow);
    if (!isMainWindow) {
        window->setAttribute(Qt::WA_DeleteOnClose);
    }
    if (isMainWindow && serverProxy_) {
        serverProxy_->startConnectivity();
    }
    return window;
}

bool SebSession::applyProxyAuthentication(const QString &proxyHost, QAuthenticator *authenticator) const
{
    if (!authenticator) {
        return false;
    }

    for (const auto &proxy : configuration_.settings.Browser.Proxy.Proxies) {
        if (proxy.RequiresAuthentication &&
            proxy.Host.compare(proxyHost, Qt::CaseInsensitive) == 0) {
            authenticator->setUser(proxy.Username);
            authenticator->setPassword(proxy.Password);
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
    switch (configuration_.settings.Browser.PopupPolicy) {
    case seb::settings::browser::PopupPolicy::Allow:
        return true;
    case seb::settings::browser::PopupPolicy::AllowSameHost:
        return sameHost;
    case seb::settings::browser::PopupPolicy::AllowSameHostAndWindow:
        if (openInSameWindow) {
            *openInSameWindow = true;
        }
        return sameHost;
    case seb::settings::browser::PopupPolicy::AllowSameWindow:
        if (openInSameWindow) {
            *openInSameWindow = true;
        }
        return true;
    case seb::settings::browser::PopupPolicy::Block:
        return false;
    }

    return false;
}

bool SebSession::isQuitUrl(const QUrl &url) const
{
    QString configured = configuration_.settings.Browser.QuitUrl;
    if (configured.trimmed().isEmpty()) {
        return false;
    }

    while (configured.endsWith(QLatin1Char('/'))) {
        configured.chop(1);
    }
    const QString pattern = QRegularExpression::escape(configured);
    const QRegularExpression matcher(
        QStringLiteral("^%1/?$").arg(pattern),
        QRegularExpression::CaseInsensitiveOption);
    return matcher.match(url.toString(QUrl::FullyEncoded)).hasMatch();
}

bool SebSession::promptForHomeNavigation(QWidget *parent) const
{
    if (!configuration_.settings.Browser.HomeNavigationMessage.isEmpty()) {
        const auto answer = QMessageBox::question(
            parent,
            QStringLiteral("Navigate Home"),
            configuration_.settings.Browser.HomeNavigationMessage,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            return false;
        }
    }

    if (configuration_.settings.Browser.HomeNavigationRequiresPassword &&
        !configuration_.settings.Security.QuitPasswordHash.isEmpty()) {
        return promptForPassword(
            parent,
            QStringLiteral("Quit Password Required"),
            QStringLiteral("Enter the quit password to navigate home."));
    }

    return true;
}

bool SebSession::requestApplicationQuit(QWidget *parent, const QString &reason) const
{
    if (!configuration_.settings.Security.AllowTermination) {
        QMessageBox::warning(
            parent,
            QStringLiteral("Termination Disabled"),
            QStringLiteral("Termination is disabled by the active configuration."));
        return false;
    }

    if (!configuration_.settings.Security.QuitPasswordHash.isEmpty()) {
        return promptForPassword(parent, QStringLiteral("Quit Password Required"), reason);
    }

    return true;
}

const seb::settings::AppSettings &SebSession::settings() const
{
    return configuration_.settings;
}

const seb::configuration::contracts::SessionConfiguration &SebSession::configuration() const
{
    return configuration_;
}

seb::configuration::contracts::cryptography::IKeyGenerator *SebSession::keyGenerator() const
{
    return keyGenerator_.get();
}

seb::browser::contracts::IWebProfile *SebSession::profile() const
{
    return profilePtr_;
}

seb::browser::contracts::IEngineProvider *SebSession::engineProvider() const
{
    return engineProviderPtr_;
}

QUrl SebSession::homeUrl() const
{
    return QUrl(configuration_.settings.Browser.HomeUrl);
}

QUrl SebSession::initialUrl() const
{
    QUrl url(configuration_.settings.Browser.StartUrl);
    if (!url.isValid()) {
        return url;
    }

    if (configuration_.settings.Browser.UseQueryParameter && !configuration_.settings.Browser.StartUrlQuery.isEmpty()) {
        const QString query = configuration_.settings.Browser.StartUrlQuery.startsWith('?')
            ? configuration_.settings.Browser.StartUrlQuery.mid(1)
            : configuration_.settings.Browser.StartUrlQuery;

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

QString SebSession::downAndUploadDirectory() const
{
    return defaultDownloadDirectory();
}

bool SebSession::isReconfigurationInProgress() const
{
    return reconfigurationDownloadId_.has_value();
}

bool SebSession::isMainNavigationAllowed(const QUrl &url) const
{
    if (!interceptor_) {
        return true;
    }

    return interceptor_->evaluate(
               url,
               seb::browser::contracts::ResourceType::MainFrame) !=
           seb::browser::FilterDecision::Block;
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

void SebSession::handleDownloadRequested(seb::browser::contracts::DownloadRequest &request)
{
    if (request.id.isNull()) {
        request.id = QUuid::createUuid();
    }

    QString fileName = QFileInfo(request.suggestedFileName).fileName();
    if (fileName.isEmpty() || fileName == QStringLiteral(".") || fileName == QStringLiteral("..")) {
        fileName = QStringLiteral("download");
    }
    const bool sebConfig = seb::browser::ConfigurationResourceDetector::isConfigurationResource(
        request.url, fileName, request.mimeType);

    if (sebConfig && !fileName.endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive)) {
        fileName += QStringLiteral(".seb");
    }

    if (!configuration_.settings.Browser.AllowDownloads && !sebConfig) {
        return;
    }

    if (sebConfig && !configuration_.settings.Browser.AllowConfigurationDownloads) {
        return;
    }

    if (sebConfig) {
        if (!isConfigurationReconfigurationAllowed(request.url) ||
            reconfigurationDownloadId_.has_value()) {
            qWarning() << "Blocked SEB configuration download:" << request.url;
            return;
        }

        if (!configurationDownloadDirectory_ || !configurationDownloadDirectory_->isValid()) {
            configurationDownloadDirectory_ = std::make_unique<QTemporaryDir>(
                QDir::tempPath() + QStringLiteral("/seb-linux-configuration-XXXXXX"));
        }

        if (!configurationDownloadDirectory_ || !configurationDownloadDirectory_->isValid()) {
            return;
        }

        reconfigurationDownloadId_ = request.id;
        request.targetDirectory = configurationDownloadDirectory_->path();
        request.targetFileName = fileName;
        request.accepted = true;
        return;
    }

    QString destination = QDir(defaultDownloadDirectory()).filePath(fileName);
    if (configuration_.settings.Browser.AllowCustomDownAndUploadLocation) {
        const QString selected = QFileDialog::getSaveFileName(
            QApplication::activeWindow(),
            QStringLiteral("Save Download"),
            destination);

        if (selected.isEmpty()) {
            return;
        }

        destination = selected;
    }

    if (!configuration_.settings.Browser.AllowCustomDownAndUploadLocation) {
        QFileInfo candidate(destination);
        const QString directory = candidate.dir().path();
        const QString stem = candidate.completeBaseName();
        const QString suffix = candidate.completeSuffix();
        int index = 0;
        while (candidate.exists()) {
            ++index;
            const QString numbered = suffix.isEmpty()
                ? QStringLiteral("%1(%2)").arg(stem).arg(index)
                : QStringLiteral("%1(%2).%3").arg(stem).arg(index).arg(suffix);
            destination = QDir(directory).filePath(numbered);
            candidate.setFile(destination);
        }
    }

    const QFileInfo info(destination);
    request.targetDirectory = info.dir().path();
    request.targetFileName = info.fileName();
    request.accepted = true;
}

bool SebSession::isConfigurationReconfigurationAllowed(const QUrl &url) const
{
    const auto &security = configuration_.settings.Security;
    if (security.QuitPasswordHash.isEmpty()) {
        return configuration_.settings.ConfigurationMode ==
                   seb::settings::ConfigurationMode::ConfigureClient ||
               security.AllowReconfiguration;
    }

    if (!security.AllowReconfiguration || security.ReconfigurationUrl.trimmed().isEmpty()) {
        return false;
    }

    QString pattern = QRegularExpression::escape(security.ReconfigurationUrl);
    pattern.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
    const QRegularExpression matcher(
        QStringLiteral("^%1$").arg(pattern),
        QRegularExpression::CaseInsensitiveOption);
    QUrl equivalent = url;
    if (url.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) == 0) {
        equivalent.setScheme(QStringLiteral("seb"));
    } else if (url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0) {
        equivalent.setScheme(QStringLiteral("sebs"));
    } else {
        equivalent = seb::browser::ConfigurationResourceDetector::downloadUrl(url);
    }

    return matcher.match(url.toString(QUrl::FullyEncoded)).hasMatch() ||
           matcher.match(equivalent.toString(QUrl::FullyEncoded)).hasMatch();
}

QString SebSession::buildUserAgent() const
{
    const QString customUserAgent =
        configuration_.settings.Browser.CustomUserAgent.trimmed();
    QString agent;

    if (configuration_.settings.Browser.UseCustomUserAgent && !customUserAgent.isEmpty()) {
        agent = customUserAgent;
    } else {
        agent = profilePtr_ ? profilePtr_->defaultHttpUserAgent().trimmed() : QString();
    }

    appendUserAgentPart(
        agent,
        QStringLiteral("SEB-Linux/") + configuration_.appConfig.programBuildVersion);
    appendUserAgentPart(
        agent,
        QStringLiteral("SEB/") + configuration_.appConfig.sebCompatibilityVersion);
    appendUserAgentPart(agent, configuration_.settings.Browser.UserAgentSuffix);

    return agent.trimmed();
}

QString SebSession::defaultDownloadDirectory() const
{
    if (downloadDirectory_ && downloadDirectory_->isValid()) {
        return downloadDirectory_->path();
    }

    const QString configured = configuration_.settings.Browser.DownAndUploadDirectory.trimmed();
    if (!configured.isEmpty() && QDir(configured).exists()) {
        return configured;
    }

    if (!configured.isEmpty()) {
        qWarning() << "Configured SEB download directory does not exist; using Downloads:" << configured;
    }

    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (path.isEmpty()) {
        path = QDir::homePath();
    }
    return path;
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
    if (hash.compare(configuration_.settings.Security.QuitPasswordHash, Qt::CaseInsensitive) == 0) {
        return true;
    }

    QMessageBox::warning(parent, title, QStringLiteral("The entered quit password is invalid."));
    return false;
}
