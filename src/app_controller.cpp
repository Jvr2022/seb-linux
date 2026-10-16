#include "app_controller.h"

#include "browser_window.h"
#include "browser/engines/engine_factory.h"
#include "browser/webengine_environment.h"
#include "browser/contracts/i_engine_provider.h"
#include "browser/contracts/i_webprofile.h"
#include "configuration/cryptography/hash_algorithm.h"
#include "runtime/version_restriction_validator.h"
#include "server/data/exam.h"
#include "server/server_proxy.h"
#include "seb_session.h"
#include "ui/runtime_window.h"

#include <QCoreApplication>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QListWidget>
#include <QVBoxLayout>
#include <QTimer>
#include <QTemporaryDir>
#include <QTextStream>

namespace {

QUrl resourceUrl(const QString &resource)
{
    const QFileInfo file(resource);
    return file.exists() ? QUrl::fromLocalFile(file.absoluteFilePath()) : QUrl::fromUserInput(resource);
}

bool hasArgument(const QStringList &arguments, const QString &name)
{
    return arguments.contains(name);
}

void applyRuntimeOverrides(seb::settings::AppSettings &settings)
{
    const QStringList arguments = QCoreApplication::arguments();
    const int urlIndex = arguments.indexOf(QStringLiteral("--url"));
    const int shortUrlIndex = arguments.indexOf(QStringLiteral("-u"));
    const int selectedUrlIndex = urlIndex >= 0 ? urlIndex : shortUrlIndex;
    if (selectedUrlIndex >= 0 && selectedUrlIndex + 1 < arguments.size()) {
        settings.Browser.StartUrl = arguments.at(selectedUrlIndex + 1).trimmed();
    }

    if (hasArgument(arguments, QStringLiteral("--show-toolbar"))) {
        settings.Browser.MainWindow.ShowToolbar = true;
    }
    if (hasArgument(arguments, QStringLiteral("--allow-address-bar"))) {
        settings.Browser.MainWindow.AllowAddressBar = true;
        settings.Browser.MainWindow.ShowToolbar = true;
    }
    if (hasArgument(arguments, QStringLiteral("--allow-navigation"))) {
        settings.Browser.MainWindow.AllowBackwardNavigation = true;
        settings.Browser.MainWindow.AllowForwardNavigation = true;
    }
    if (hasArgument(arguments, QStringLiteral("--allow-reload"))) {
        settings.Browser.MainWindow.AllowReloading = true;
        settings.Browser.MainWindow.ShowReloadButton = true;
    }
    if (hasArgument(arguments, QStringLiteral("--allow-devtools"))) {
        settings.Browser.MainWindow.AllowDeveloperConsole = true;
    }
    if (hasArgument(arguments, QStringLiteral("--windowed"))) {
        settings.Browser.MainWindow.FullScreenMode = false;
    }
    if (hasArgument(arguments, QStringLiteral("--fullscreen"))) {
        settings.Browser.MainWindow.FullScreenMode = true;
    }
    if (hasArgument(arguments, QStringLiteral("--disable-quit"))) {
        settings.Security.AllowTermination = false;
    }
}

QString loadStatusMessage(seb::configuration::contracts::LoadStatus status)
{
    using seb::configuration::contracts::LoadStatus;
    switch (status) {
    case LoadStatus::InvalidData:
        return QStringLiteral("The SEB configuration contains invalid data.");
    case LoadStatus::LoadWithBrowser:
        return QStringLiteral("The configuration resource must be opened in the browser.");
    case LoadStatus::NotSupported:
        return QStringLiteral("The configuration resource format is not supported.");
    case LoadStatus::PasswordNeeded:
        return QStringLiteral("The SEB configuration requires a password.");
    case LoadStatus::UnexpectedError:
        return QStringLiteral("An unexpected error occurred while loading the SEB configuration.");
    case LoadStatus::Success:
        return {};
    }
    return QStringLiteral("The SEB configuration could not be loaded.");
}

void applyStartUrlQuery(
    const QUrl &resource,
    seb::settings::AppSettings &settings)
{
    const QString query = resource.query(QUrl::FullyEncoded);
    const int separator = query.lastIndexOf(QLatin1Char('?'));
    if (separator > 0) {
        settings.Browser.StartUrlQuery = query.mid(separator);
    }
}

bool samePasswordHash(const QString &left, const QString &right)
{
    return !left.isEmpty() && left.compare(right, Qt::CaseInsensitive) == 0;
}

bool validateVersionRestrictions(
    const seb::configuration::contracts::SessionConfiguration &configuration,
    QString *error)
{
    const auto &restrictions = configuration.settings.Security.VersionRestrictions;
    const seb::runtime::VersionRestrictionValidator validator;
    if (validator.validate(
            restrictions,
            configuration.appConfig.sebCompatibilityBuildVersion,
            false)) {
        return true;
    }

    if (error) {
        *error = QStringLiteral(
                     "This configuration requires another Safe Exam Browser version.\n"
                     "Installed Linux SEB: %1\n"
                     "SEB compatibility level: %2")
                     .arg(configuration.appConfig.programBuildVersion,
                          configuration.appConfig.sebCompatibilityBuildVersion);
    }
    return false;
}

QString persistentClientPath(const seb::configuration::contracts::AppConfig &config)
{
    if (QFileInfo::exists(config.programDataFilePath)) {
        return config.programDataFilePath;
    }
    if (QFileInfo::exists(config.appDataFilePath)) {
        return config.appDataFilePath;
    }
    return {};
}

bool chooseServerExam(
    QWidget *parent,
    const QList<seb::server::data::Exam> &exams,
    seb::server::data::Exam &selected)
{
    if (exams.size() == 1) {
        selected = exams.first();
        return true;
    }

    QDialog dialog(parent);
    dialog.setWindowTitle(QStringLiteral("Select exam"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *list = new QListWidget(&dialog);
    for (const auto &exam : exams) {
        auto *item = new QListWidgetItem(
            exam.name + (exam.lmsName.isEmpty()
                ? QString()
                : QStringLiteral(" (%1)").arg(exam.lmsName)),
            list);
        item->setData(Qt::UserRole, exam.id);
    }
    list->setCurrentRow(0);
    layout->addWidget(list);
    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal,
        &dialog);
    layout->addWidget(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    if (dialog.exec() != QDialog::Accepted || !list->currentItem()) {
        return false;
    }
    const QString id = list->currentItem()->data(Qt::UserRole).toString();
    for (const auto &exam : exams) {
        if (exam.id == id) {
            selected = exam;
            return true;
        }
    }
    return false;
}

enum class ServerFailureAction { Retry, Fallback, Abort };

ServerFailureAction chooseServerFailure(
    QWidget *parent,
    const QString &message,
    bool allowFallback)
{
    QMessageBox box(parent);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(QStringLiteral("SEB Server unavailable"));
    box.setText(message);
    auto *retry = box.addButton(QStringLiteral("Retry"), QMessageBox::AcceptRole);
    QPushButton *fallback = nullptr;
    if (allowFallback) {
        fallback = box.addButton(QStringLiteral("Use fallback"), QMessageBox::DestructiveRole);
    }
    auto *abort = box.addButton(QStringLiteral("Abort"), QMessageBox::RejectRole);
    box.setDefaultButton(retry);
    box.exec();
    if (box.clickedButton() == retry) return ServerFailureAction::Retry;
    if (fallback && box.clickedButton() == fallback) return ServerFailureAction::Fallback;
    Q_UNUSED(abort);
    return ServerFailureAction::Abort;
}

bool tryServerFallbackPassword(
    QWidget *parent,
    const QString &expectedHash,
    QString &passwordOut)
{
    if (expectedHash.isEmpty()) {
        return false;
    }
    const seb::configuration::cryptography::HashAlgorithm hashAlgorithm;
    for (int attempt = 0; attempt < 5; ++attempt) {
        bool accepted = false;
        const QString password = QInputDialog::getText(
            parent,
            QStringLiteral("Fallback password"),
            QStringLiteral("Enter the password for the offline fallback configuration."),
            QLineEdit::Password,
            QString(),
            &accepted);
        if (!accepted) {
            return false;
        }
        if (hashAlgorithm.generateHashFor(password)
                .compare(expectedHash, Qt::CaseInsensitive) == 0) {
            passwordOut = password;
            return true;
        }
    }
    return false;
}

class ReconfigurationGuard
{
public:
    ReconfigurationGuard(bool &lock, bool active)
        : lock_(lock)
        , active_(active)
    {
        if (active_) {
            lock_ = true;
        }
    }

    ~ReconfigurationGuard()
    {
        if (active_) {
            lock_ = false;
        }
    }

private:
    bool &lock_;
    bool active_;
};

}  // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    sessionConfiguration_ = repository_.initializeSessionConfiguration();
}

AppController::~AppController() = default;

bool AppController::loadSettingsWithPassword(
    const QUrl &resource,
    seb::settings::AppSettings &settings,
    seb::configuration::contracts::cryptography::PasswordParameters &password,
    const QString &currentPasswordHash,
    bool localSettings,
    seb::configuration::contracts::LoadStatus &status,
    bool &aborted)
{
    password.password = QStringLiteral("");
    password.isHash = true;
    aborted = false;
    status = repository_.tryLoadSettings(resource, settings, &password);

    if (status == seb::configuration::contracts::LoadStatus::PasswordNeeded &&
        !currentPasswordHash.isEmpty()) {
        password.password = currentPasswordHash;
        password.isHash = true;
        status = repository_.tryLoadSettings(resource, settings, &password);
    }

    for (int attempt = 0;
         attempt < 5 && status == seb::configuration::contracts::LoadStatus::PasswordNeeded;
         ++attempt) {
        bool accepted = false;
        const QString passwordText = QInputDialog::getText(
            mainWindow_,
            localSettings
                ? QStringLiteral("Local SEB Settings Password")
                : QStringLiteral("SEB Password Required"),
            localSettings
                ? QStringLiteral("Enter the password for the local SEB settings.")
                : QStringLiteral("Enter the password for this SEB configuration file."),
            QLineEdit::Password,
            QString(),
            &accepted);
        if (!accepted) {
            aborted = true;
            return false;
        }

        password.password = passwordText;
        password.isHash = false;
        status = repository_.tryLoadSettings(resource, settings, &password);
    }

    return status == seb::configuration::contracts::LoadStatus::Success ||
           status == seb::configuration::contracts::LoadStatus::LoadWithBrowser;
}

bool AppController::ensureBrowserProfile()
{
    if (browserProfileDirectory_ && browserProfileDirectory_->isValid()) {
        return true;
    }

    browserProfileDirectory_ = std::make_unique<QTemporaryDir>(
        QDir::tempPath() + QStringLiteral("/seb-linux-profile-XXXXXX"));
    return browserProfileDirectory_ && browserProfileDirectory_->isValid();
}

bool AppController::ensureBrowserProfileContext()
{
    if (browserEngineProvider_ && browserProfile_) {
        return true;
    }

    if (!ensureBrowserProfile()) {
        return false;
    }

    browserEngineProvider_ = seb::browser::createEngineProvider();
    if (!browserEngineProvider_) {
        return false;
    }

    const QString storageName = QStringLiteral("seb-browser-resource-") +
        QUuid::createUuid().toString(QUuid::WithoutBraces);
    browserProfile_ = browserEngineProvider_->createProfile(this, storageName);
    return browserProfile_ != nullptr;
}

bool AppController::initializeServerSession(
    seb::settings::AppSettings &settings,
    QString *error)
{
    using seb::configuration::contracts::LoadStatus;
    using seb::settings::SessionMode;

    if (settings.SessionMode != SessionMode::Server) {
        return true;
    }

    if (serverProxy_ && serverProxy_->isConnected() && !serverProxy_->examId().isEmpty()) {
        const auto info = serverProxy_->connectionInfo();
        sessionConfiguration_.appConfig.serverApi = info.api;
        sessionConfiguration_.appConfig.serverConnectionToken = info.connectionToken;
        sessionConfiguration_.appConfig.serverOauth2Token = info.oauth2Token;
        sessionConfiguration_.appConfig.serverExamId = serverProxy_->examId();
        return true;
    }

    if (settings.Server.ServerUrl.trimmed().isEmpty()) {
        if (error) *error = QStringLiteral("SEB Server mode requires sebServerURL.");
        return false;
    }

    const auto connectionBrowser = settings.Browser;
    const auto connectionServer = settings.Server;
    const auto fallbackToLocalSettings = [&]() {
        QString fallbackPassword;
        if (!tryServerFallbackPassword(
                mainWindow_,
                connectionServer.FallbackPasswordHash,
                fallbackPassword)) {
            return false;
        }
        const QString fallbackPath = persistentClientPath(sessionConfiguration_.appConfig);
        if (fallbackPath.isEmpty()) {
            return false;
        }
        seb::settings::AppSettings fallbackSettings;
        seb::configuration::contracts::cryptography::PasswordParameters fallbackParameters;
        fallbackParameters.password = fallbackPassword;
        fallbackParameters.isHash = false;
        seb::configuration::contracts::LoadStatus fallbackStatus =
            seb::configuration::contracts::LoadStatus::UnexpectedError;
        bool fallbackAborted = false;
        const bool loaded = loadSettingsWithPassword(
            QUrl::fromLocalFile(fallbackPath),
            fallbackSettings,
            fallbackParameters,
            QString(),
            true,
            fallbackStatus,
            fallbackAborted);
        if (!loaded || fallbackStatus != seb::configuration::contracts::LoadStatus::Success) {
            return false;
        }
        fallbackSettings.SessionMode = SessionMode::Normal;
        settings = fallbackSettings;
        serverProxy_.reset();
        return true;
    };
    if (!serverProxy_) {
        serverProxy_ = std::make_unique<seb::server::ServerProxy>(
            sessionConfiguration_.appConfig,
            this);
    }

    for (int attempt = 0; attempt < 5; ++attempt) {
        serverProxy_->initialize(connectionServer);
        const auto connected = serverProxy_->connectToServer();
        if (!connected.success) {
            const auto action = chooseServerFailure(
                mainWindow_, connected.message, connectionServer.PerformFallback);
            if (action == ServerFailureAction::Retry) continue;
            if (action == ServerFailureAction::Fallback && connectionServer.PerformFallback) {
                if (fallbackToLocalSettings()) {
                    return true;
                }
            }
            if (error) *error = connected.message;
            return false;
        }

        QList<seb::server::data::Exam> exams;
        const auto available = serverProxy_->availableExams(exams, connectionServer.ExamId);
        if (!available.success) {
            const auto action = chooseServerFailure(
                mainWindow_, available.message, connectionServer.PerformFallback);
            if (action == ServerFailureAction::Retry) continue;
            if (action == ServerFailureAction::Fallback && connectionServer.PerformFallback) {
                if (fallbackToLocalSettings()) {
                    return true;
                }
            }
            if (error) *error = available.message;
            return false;
        }

        seb::server::data::Exam selected;
        if (!connectionServer.ExamId.isEmpty()) {
            for (const auto &exam : exams) {
                if (exam.id == connectionServer.ExamId) {
                    selected = exam;
                    break;
                }
            }
            if (selected.id.isEmpty()) {
                if (error) *error = QStringLiteral("The configured SEB Server exam was not available.");
                return false;
            }
        } else if (!chooseServerExam(mainWindow_, exams, selected)) {
            if (error) *error = QStringLiteral("No SEB Server exam was selected.");
            return false;
        }

        const QString directory = sessionConfiguration_.appConfig.temporaryDirectory;
        if (!QDir().mkpath(directory)) {
            if (error) *error = QStringLiteral("Could not create temporary SEB Server storage.");
            return false;
        }
        const QString path = QDir(directory).filePath(
            QStringLiteral("seb-server-%1.seb").arg(QUuid::createUuid().toString(QUuid::WithoutBraces)));
        const auto downloaded = serverProxy_->downloadExamConfiguration(selected, path);
        if (!downloaded.success) {
            QFile::remove(path);
            if (error) *error = downloaded.message;
            return false;
        }

        seb::settings::AppSettings examSettings;
        seb::configuration::contracts::cryptography::PasswordParameters password;
        LoadStatus status = LoadStatus::UnexpectedError;
        bool aborted = false;
        const bool loaded = loadSettingsWithPassword(
            QUrl::fromLocalFile(path),
            examSettings,
            password,
            connectionServer.FallbackPasswordHash,
            false,
            status,
            aborted);
        QFile::remove(path);
        if (!loaded || status != LoadStatus::Success) {
            if (error) {
                *error = aborted
                    ? QStringLiteral("The SEB Server exam configuration password was not accepted.")
                    : loadStatusMessage(status);
            }
            return false;
        }

        QString browserExamKey;
        const auto selectedResponse = serverProxy_->selectExam(selected, &browserExamKey);
        if (!selectedResponse.success) {
            if (error) *error = selectedResponse.message;
            return false;
        }

        examSettings.Browser.StartUrl = selected.url;
        examSettings.Browser.StartUrlQuery = connectionBrowser.StartUrlQuery;
        examSettings.Server = connectionServer;
        examSettings.SessionMode = SessionMode::Server;
        if (!browserExamKey.isEmpty()) {
            examSettings.Browser.CustomBrowserExamKey = browserExamKey;
        }
        settings = examSettings;

        const auto info = serverProxy_->connectionInfo();
        sessionConfiguration_.appConfig.serverApi = info.api;
        sessionConfiguration_.appConfig.serverConnectionToken = info.connectionToken;
        sessionConfiguration_.appConfig.serverOauth2Token = info.oauth2Token;
        sessionConfiguration_.appConfig.serverExamId = selected.id;
        return true;
    }

    if (error) *error = QStringLiteral("Could not connect to the SEB Server.");
    return false;
}

bool AppController::launch(const QString &resource, QString *error)
{
    const bool isReconfiguration = session_ != nullptr;
    if (isReconfiguration &&
        (reconfigurationInProgress_ ||
         (session_ && session_->isReconfigurationInProgress()))) {
        if (error) {
            *error = QStringLiteral("A Safe Exam Browser reconfiguration is already in progress.");
        }
        return false;
    }
    ReconfigurationGuard reconfigurationGuard(reconfigurationInProgress_, isReconfiguration);
    if (session_) {
        serverProxy_ = session_->takeServerProxy();
    }
    const bool previousBrowserResource = sessionConfiguration_.isBrowserResource;
    const QString previousBrowserProfilePath = sessionConfiguration_.browserProfilePath;
    const QString previousAdminPasswordHash = session_
        ? session_->settings().Security.AdminPasswordHash
        : QString();
    sessionConfiguration_ = repository_.initializeSessionConfiguration();
    if (previousBrowserResource && !previousBrowserProfilePath.isEmpty()) {
        sessionConfiguration_.browserProfilePath = previousBrowserProfilePath;
        sessionConfiguration_.reuseBrowserProfile = true;
    }
    const QStringList warnings;
    const bool hasRequestedResource = !resource.trimmed().isEmpty();

    QUrl targetResource;
    if (hasRequestedResource) {
        targetResource = resourceUrl(resource);
    } else {
        const QString path = persistentClientPath(sessionConfiguration_.appConfig);
        if (!path.isEmpty()) {
            targetResource = QUrl::fromLocalFile(path);
        }
    }

    if (!targetResource.isValid() || targetResource.isEmpty()) {
        sessionConfiguration_.settings = repository_.loadDefaultSettings();
    } else {
        auto &settings = sessionConfiguration_.settings;
        seb::configuration::contracts::cryptography::PasswordParameters passwordUsed;
        seb::configuration::contracts::LoadStatus status;
        bool passwordAborted = false;
        QString currentAdminPasswordHash;

        if (hasRequestedResource && !isReconfiguration) {
            const QString clientPath = persistentClientPath(sessionConfiguration_.appConfig);
            if (!clientPath.isEmpty()) {
                seb::settings::AppSettings clientSettings;
                const bool loaded = loadSettingsWithPassword(
                    QUrl::fromLocalFile(clientPath),
                    clientSettings,
                    passwordUsed,
                    QString(),
                    true,
                    status,
                    passwordAborted);
                if (!loaded) {
                    if (error) {
                        *error = passwordAborted
                            ? QStringLiteral("The local SEB settings password was not accepted.")
                            : loadStatusMessage(status);
                    }
                    return false;
                }
                currentAdminPasswordHash = clientSettings.Security.AdminPasswordHash;
            }
        }

        if (currentAdminPasswordHash.isEmpty()) {
            currentAdminPasswordHash = previousAdminPasswordHash;
        }

        const bool loaded = loadSettingsWithPassword(
            targetResource,
            settings,
            passwordUsed,
            currentAdminPasswordHash,
            !hasRequestedResource,
            status,
            passwordAborted);

        if (!loaded && status == seb::configuration::contracts::LoadStatus::PasswordNeeded) {
            if (error) {
                *error = passwordAborted
                    ? QStringLiteral("The SEB configuration password was not accepted.")
                    : QStringLiteral("The SEB configuration password was not accepted after five attempts.");
            }
            return false;
        }

        if (status == seb::configuration::contracts::LoadStatus::LoadWithBrowser) {
            sessionConfiguration_.isBrowserResource = true;
            if (sessionConfiguration_.browserProfilePath.isEmpty()) {
                if (!previousBrowserResource) {
                    browserProfile_.reset();
                    browserEngineProvider_.reset();
                    browserProfileDirectory_.reset();
                }
                if (!ensureBrowserProfile()) {
                    if (error) {
                        *error = QStringLiteral("Could not create temporary browser profile storage.");
                    }
                    return false;
                }
                if (!ensureBrowserProfileContext()) {
                    if (error) {
                        *error = QStringLiteral("Could not create a persistent browser profile.");
                    }
                    return false;
                }
                sessionConfiguration_.browserProfilePath = browserProfileDirectory_->path();
                sessionConfiguration_.reuseBrowserProfile = true;
            }
            settings.Applications.Blacklist.clear();
            settings.Applications.Whitelist.clear();
            settings.Browser.DeleteCacheOnShutdown = false;
            settings.Browser.DeleteCookiesOnShutdown = false;
            settings.Browser.StartUrl = targetResource.toString();
            settings.Display.AllowedDisplays = 10;
            settings.Display.IgnoreError = true;
            settings.Display.InternalDisplayOnly = false;
            settings.Security.AllowReconfiguration = true;
            settings.Security.VirtualMachinePolicy = seb::settings::security::VirtualMachinePolicy::Allow;
            settings.Service.IgnoreService = true;
            settings.UserInterface.ActionCenter.EnableActionCenter = false;
        } else if (status != seb::configuration::contracts::LoadStatus::Success) {
            if (error) {
                *error = loadStatusMessage(status);
            }
            return false;
        }

        if (status == seb::configuration::contracts::LoadStatus::Success ||
            status == seb::configuration::contracts::LoadStatus::LoadWithBrowser) {
            applyStartUrlQuery(targetResource, settings);
            if (previousBrowserResource) {
                settings.Browser.DeleteCookiesOnStartup = false;
            }
        }

        if (hasRequestedResource &&
            status == seb::configuration::contracts::LoadStatus::Success &&
            settings.ConfigurationMode == seb::settings::ConfigurationMode::ConfigureClient) {
            const bool currentPasswordRequired = !currentAdminPasswordHash.isEmpty();
            bool authenticated = !currentPasswordRequired;

            if (currentPasswordRequired) {
                const QString incomingAdminHash = settings.Security.AdminPasswordHash;
                const bool sameAdminPassword = samePasswordHash(
                    currentAdminPasswordHash,
                    incomingAdminHash);

                QString suppliedHash;
                if (!passwordUsed.password.isEmpty()) {
                    suppliedHash = passwordUsed.isHash
                        ? passwordUsed.password
                        : seb::configuration::cryptography::HashAlgorithm().generateHashFor(passwordUsed.password);
                }

                authenticated = sameAdminPassword ||
                                samePasswordHash(currentAdminPasswordHash, suppliedHash);

                if (!authenticated) {
                    seb::configuration::cryptography::HashAlgorithm hashAlgorithm;
                    for (int attempt = 0; attempt < 5 && !authenticated; ++attempt) {
                        bool accepted = false;
                        const QString password = QInputDialog::getText(
                            mainWindow_,
                            QStringLiteral("Administrator Password Required"),
                            QStringLiteral("Enter the current SEB administrator password to replace the client configuration."),
                            QLineEdit::Password,
                            QString(),
                            &accepted);
                        if (!accepted) {
                            break;
                        }
                        authenticated = samePasswordHash(
                            currentAdminPasswordHash,
                            hashAlgorithm.generateHashFor(password));
                    }
                }
            }

            if (!authenticated) {
                if (error) {
                    *error = QStringLiteral("The current SEB administrator password was not accepted.");
                }
                return false;
            }

            const auto saveStatus = repository_.configureClientWith(targetResource, &passwordUsed);
            if (saveStatus != seb::configuration::contracts::SaveStatus::Success) {
                if (error) {
                    *error = saveStatus == seb::configuration::contracts::SaveStatus::InvalidData
                        ? QStringLiteral("Client configuration contains invalid data.")
                        : saveStatus == seb::configuration::contracts::SaveStatus::NotSupported
                            ? QStringLiteral("Client configuration format is not supported.")
                            : QStringLiteral("Unexpected error while saving client configuration.");
                }
                return false;
            }

            if (!session_) {
                const auto answer = QMessageBox::question(
                    mainWindow_,
                    QStringLiteral("Client Configuration Saved"),
                    QStringLiteral("The client configuration was saved successfully. Continue starting Safe Exam Browser?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes);
                if (answer == QMessageBox::No) {
                    if (error) *error = QStringLiteral("Client configuration saved successfully.");
                    return false;
                }
            }
        }
    }

    if (!initializeServerSession(sessionConfiguration_.settings, error)) {
        return false;
    }

    if (!validateVersionRestrictions(sessionConfiguration_, error)) {
        return false;
    }

    applyRuntimeOverrides(sessionConfiguration_.settings);
    return applySettings(sessionConfiguration_, warnings, error);
}

bool AppController::applySettings(
    const seb::configuration::contracts::SessionConfiguration &configuration,
    const QStringList &warnings,
    QString *error)
{
    const auto &settings = configuration.settings;
    if (!settings.Browser.EnableBrowser) {
        if (error) {
            *error = QStringLiteral("The loaded configuration disables the browser.");
        }
        return false;
    }

    QTextStream err(stderr);
    for (const QString &warning : warnings) {
        err << "warning: " << warning << Qt::endl;
    }

    seb::browser::applyWebEngineEnvironment(settings.Browser);

    if (!runtimeWindow_) {
        runtimeWindow_ = std::make_unique<RuntimeWindow>();
    }
    runtimeWindow_->setVersionText(
        QCoreApplication::applicationVersion(),
        QCoreApplication::applicationVersion());
    runtimeWindow_->setStatusText(QStringLiteral("Starting Safe Exam Browser"));
    runtimeWindow_->show();
    runtimeWindow_->raise();
    runtimeWindow_->activateWindow();

    QElapsedTimer startupTimer;
    startupTimer.start();

    auto nextSession = std::make_unique<SebSession>(
        configuration,
        [this](const QString &resource, QWidget *parent) {
            QString error;
            if (launch(resource, &error)) {
                return true;
            }

            QMessageBox::critical(parent, QStringLiteral("Failed to Open SEB Resource"), error);
            return false;
        },
        this,
        configuration.reuseBrowserProfile ? browserEngineProvider_.get() : nullptr,
        configuration.reuseBrowserProfile ? browserProfile_.get() : nullptr,
        std::move(serverProxy_));

    BrowserWindow *nextWindow = nextSession->createWindow(nextSession->initialUrl(), true);
    runtimeWindow_->setStatusText(QStringLiteral("Opening secure browser window"));

    const qint64 remaining = 5000 - startupTimer.elapsed();
    if (remaining > 0) {
        QEventLoop loop;
        QTimer::singleShot(static_cast<int>(remaining), &loop, &QEventLoop::quit);
        loop.exec();
    }

    nextWindow->show();
    runtimeWindow_->hide();

    if (mainWindow_) {
        mainWindow_->hide();
        mainWindow_->deleteLater();
        mainWindow_ = nullptr;
    }

    if (session_) {
        session_.release()->deleteLater();
    }

    mainWindow_ = nextWindow;
    session_ = std::move(nextSession);
    return true;
}
