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
#include "security/security_service.h"
#include "security/session_integrity_store.h"
#include "ui/runtime_window.h"

#include <QCoreApplication>
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFile>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QApplication>
#include <QVBoxLayout>
#include <QTimer>
#include <QTemporaryDir>
#include <QTextStream>
#include <QStandardPaths>
#include <QPointer>

namespace {

QUrl resourceUrl(const QString &resource)
{
    const QFileInfo file(resource);
    return file.exists() ? QUrl::fromLocalFile(file.absoluteFilePath()) : QUrl::fromUserInput(resource);
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

class ServerWorkflowDialog : public QDialog
{
public:
    using QDialog::QDialog;
    std::function<void()> abort;
    void reject() override { if (abort) abort(); }
};

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
    qApp->installEventFilter(this);
    qApp->setQuitOnLastWindowClosed(false);
    security_ = std::make_unique<seb::security::SecurityService>(this);
    sessionIntegrityStore_ = std::make_unique<seb::security::SessionIntegrityStore>(
        sessionConfiguration_.appConfig.sessionCacheFilePath);
    connect(security_.get(), &seb::security::SecurityService::secureViolationDetected, this, [this](const QString &reason) {
        qCritical().noquote() << reason;
        shutdown();
    }, Qt::QueuedConnection);
}

AppController::~AppController() = default;

bool AppController::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp && event->type() == QEvent::Quit && !shutdownComplete_) {
        if (serverWorkflowActive_) {
            quitRequestedDuringWorkflow_ = true;
            if (abortServerWorkflow_) abortServerWorkflow_();
            return true;
        }
        QTimer::singleShot(0, this, &AppController::shutdown);
        return true;
    }
    return QObject::eventFilter(watched, event);
}

void AppController::shutdown()
{
    if (serverWorkflowActive_) {
        quitRequestedDuringWorkflow_ = true;
        if (abortServerWorkflow_) abortServerWorkflow_();
        return;
    }
    if (shuttingDown_) return;
    shuttingDown_ = true;
    security_->stopMonitoring();
    if (session_ && !serverProxy_) serverProxy_ = session_->takeServerProxy();
    if (session_) for (auto *window : session_->browserWindows()) window->setEnabled(false);
    auto finish = [this] { QTimer::singleShot(0, this, &AppController::completeShutdown); };
    if (serverProxy_) serverProxy_->disconnectFromServer([this, finish](auto response) {
        if (!response.success) sessionIntegrityCleanupAllowed_ = false;
        finish();
    });
    else finish();
}

void AppController::completeShutdown()
{
    if (auto *dialog = qobject_cast<QDialog *>(qApp->activeModalWidget())) {
        dialog->reject();
        QTimer::singleShot(0, this, &AppController::completeShutdown);
        return;
    }
    if (session_) {
        const auto windows = session_->browserWindows();
        for (auto *window : windows) delete window;
        mainWindow_ = nullptr;
        if (!browserShutdownFinalizationStarted_) {
            browserShutdownFinalizationStarted_ = true;
            const auto &configuration = session_->configuration();
            const bool discardProfile = ephemeralBrowserProfile_ && !configuration.isBrowserResource &&
                configuration.settings.Browser.DeleteCacheOnShutdown &&
                configuration.settings.Browser.DeleteCookiesOnShutdown;
            session_->finalizeBrowser([this](bool success) {
                browserShutdownFinalizationComplete_ = true;
                browserShutdownFinalizationSucceeded_ = success;
                if (!success) qWarning() << "Browser cache cleanup could not be confirmed during shutdown.";
                QTimer::singleShot(0, this, &AppController::completeShutdown);
            }, discardProfile);
            return;
        }
        if (!browserShutdownFinalizationComplete_) return;
    }
    mainWindow_ = nullptr;
    QString integrityError;
    if (!sessionIntegrityStore_->finish(
            sessionIntegrityCleanupAllowed_ && browserShutdownFinalizationSucceeded_,
            &integrityError))
        qWarning().noquote() << integrityError;
    session_.reset();
    serverProxy_.reset();
    browserProfile_.reset();
    browserEngineProvider_.reset();
    shutdownComplete_ = true;
    QCoreApplication::quit();
}

bool AppController::validateSecuritySettings(const seb::settings::AppSettings &settings, QString *error)
{
    return security_->validateSettings(settings, error);
}

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

bool AppController::ensureBrowserProfileContext(const seb::settings::browser::BrowserSettings &settings, QString *error)
{
    if (!seb::browser::applyWebEngineEnvironment(settings, error)) return false;
    if (browserEngineProvider_ && browserProfile_) return true;

    browserEngineProvider_ = seb::browser::createEngineProvider();
    if (!browserEngineProvider_) {
        return false;
    }

    const QString storageName = ephemeralBrowserProfile_ ? QString() : QStringLiteral("seb-session");
    browserProfile_ = browserEngineProvider_->createProfile(this, storageName);
    if (!browserProfile_) return false;
    if (!ephemeralBrowserProfile_) {
        const QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        browserProfile_->setPersistentStoragePath(path + QStringLiteral("/profile"));
        browserProfile_->setCachePath(path + QStringLiteral("/cache"));
    }
    return true;
}

bool AppController::finalizeServerSession(QString *error)
{
    if (!serverProxy_) return true;
    ServerWorkflowDialog finalizing(mainWindow_);
    finalizing.setWindowTitle(QStringLiteral("Disconnecting SEB Server"));
    finalizing.setWindowFlag(Qt::WindowCloseButtonHint, false);
    auto *layout = new QVBoxLayout(&finalizing);
    layout->addWidget(new QLabel(QStringLiteral("Finalizing server session…"), &finalizing));
    bool success = false;
    QTimer::singleShot(0, &finalizing, [&] {
        serverProxy_->disconnectFromServer([&](auto response) {
            success = response.success;
            if (!success && error) *error = response.message;
            finalizing.accept();
        });
    });
    serverWorkflowActive_ = true;
    finalizing.exec();
    serverWorkflowActive_ = false;
    serverProxy_.reset();
    return success;
}

bool AppController::initializeServerSession(
    seb::settings::AppSettings &settings,
    QString *error)
{
    using seb::configuration::contracts::LoadStatus;
    using seb::settings::SessionMode;

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
            seb::configuration::cryptography::HashAlgorithm().generateHashFor(fallbackPassword),
            true,
            fallbackStatus,
            fallbackAborted);
        if (!loaded || fallbackStatus != seb::configuration::contracts::LoadStatus::Success) {
            return false;
        }
        fallbackSettings.SessionMode = SessionMode::Normal;
        settings = fallbackSettings;
        return true;
    };
    if (!serverProxy_) {
        serverProxy_ = std::make_unique<seb::server::ServerProxy>(
            sessionConfiguration_.appConfig,
            this);
    }

    ServerWorkflowDialog progress(mainWindow_);
    progress.setWindowTitle(QStringLiteral("SEB Server"));
    progress.setWindowFlag(Qt::WindowCloseButtonHint, false);
    auto *layout = new QVBoxLayout(&progress);
    auto *statusLabel = new QLabel(QStringLiteral("Connecting to SEB Server…"), &progress);
    layout->addWidget(statusLabel);
    auto *cancel = new QPushButton(QStringLiteral("Abort"), &progress);
    layout->addWidget(cancel);
    bool success = false;
    bool finishing = false;
    int stage = 0;
    QList<seb::server::data::Exam> exams;
    seb::server::data::Exam selected;
    seb::settings::AppSettings examSettings;
    QTemporaryDir storage;
    const QString path = storage.filePath(QStringLiteral("exam.seb"));
    std::function<void()> next;
    std::function<void(QString, bool)> finish;
    std::function<void(seb::server::contracts::ServerResponse)> advance;
    finish = [&](QString message, bool fallback) {
        if (finishing) return;
        finishing = true;
        cancel->setEnabled(false);
        serverProxy_->disconnectFromServer([&, message, fallback](auto) {
            success = fallback && fallbackToLocalSettings();
            if (!success && error) *error = message;
            progress.accept();
        });
    };
    advance = [&](auto response) {
        if (finishing) return;
        if (response.success) { ++stage; QTimer::singleShot(0, &progress, next); return; }
        const auto action = chooseServerFailure(&progress, response.message, connectionServer.PerformFallback);
        if (action == ServerFailureAction::Retry) QTimer::singleShot(0, &progress, next);
        else finish(response.message, action == ServerFailureAction::Fallback);
    };
    next = [&] {
        if (finishing) return;
        switch (stage) {
        case 0:
            serverProxy_->connectToServer(advance); break;
        case 1:
            statusLabel->setText(QStringLiteral("Retrieving available exams…"));
            serverProxy_->availableExams([&](auto response, auto available) {
                exams = available; advance(response);
            }, connectionServer.ExamId);
            break;
        case 2:
            if (!connectionServer.ExamId.isEmpty()) {
                for (const auto &exam : exams) if (exam.id == connectionServer.ExamId) selected = exam;
            } else chooseServerExam(&progress, exams, selected);
            if (selected.id.isEmpty()) { finish(QStringLiteral("No SEB Server exam selected."), false); return; }
            ++stage; QTimer::singleShot(0, &progress, next); break;
        case 3:
            statusLabel->setText(QStringLiteral("Downloading exam configuration…"));
            if (!storage.isValid()) { finish(QStringLiteral("Could not create temporary exam storage."), false); return; }
            serverProxy_->downloadExamConfiguration(selected, path, advance); break;
        case 4: {
            seb::configuration::contracts::cryptography::PasswordParameters password;
            LoadStatus status = LoadStatus::UnexpectedError;
            bool aborted = false;
            const bool loaded = loadSettingsWithPassword(QUrl::fromLocalFile(path), examSettings,
                password, connectionServer.FallbackPasswordHash, false, status, aborted);
            QFile::remove(path);
            if (!loaded || status != LoadStatus::Success) {
                finish(aborted ? QStringLiteral("Exam configuration loading aborted.") : loadStatusMessage(status), false); return;
            }
            auto candidate = sessionConfiguration_;
            candidate.settings = examSettings;
            QString validationError;
            if (!examSettings.Browser.EnableBrowser || (!validateVersionRestrictions(candidate, &validationError) || !validateSecuritySettings(examSettings, &validationError))) {
                finish(validationError.isEmpty() ? QStringLiteral("Exam configuration disables the browser.") : validationError, false); return;
            }
            ++stage; QTimer::singleShot(0, &progress, next); break;
        }
        case 5:
            statusLabel->setText(QStringLiteral("Completing exam handshake…"));
            serverProxy_->selectExam(selected, [&](auto response, const QString &bek) {
                if (response.success) {
                    const auto invigilation = examSettings.Server.Invigilation;
                    examSettings.Server = connectionServer;
                    examSettings.Server.Invigilation = invigilation;
                    examSettings.Browser.StartUrl = selected.url;
                    examSettings.Browser.StartUrlQuery = connectionBrowser.StartUrlQuery;
                    if (!bek.isEmpty()) examSettings.Browser.CustomBrowserExamKey = bek;
                    examSettings.SessionMode = SessionMode::Server;
                    settings = examSettings;
                }
                advance(response);
            }); break;
        default:
            success = true; finishing = true; progress.accept(); break;
        }
    };
    connect(cancel, &QPushButton::clicked, &progress, [&] { finish(QStringLiteral("SEB Server startup aborted."), false); });
    progress.abort = [&] { finish(QStringLiteral("SEB Server startup aborted."), false); };
    serverProxy_->initialize(connectionServer);
    QTimer::singleShot(0, &progress, next);
    serverWorkflowActive_ = true;
    abortServerWorkflow_ = progress.abort;
    progress.exec();
    abortServerWorkflow_ = {};
    serverWorkflowActive_ = false;
    if ((!success || settings.SessionMode != SessionMode::Server) && serverProxy_) serverProxy_.reset();
    return success;
}

bool AppController::launch(const QString &resource, QString *error)
{
    if (shuttingDown_ || transitionPending_) {
        if (error) *error = QStringLiteral("Safe Exam Browser is shutting down or replacing its session.");
        return false;
    }
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
    const bool previousBrowserResource = session_ && session_->configuration().isBrowserResource;
    const QString previousAdminPasswordHash = session_
        ? session_->settings().Security.AdminPasswordHash
        : QString();
    sessionConfiguration_ = repository_.initializeSessionConfiguration();
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
            settings.Security.VerifySessionIntegrity = false;
            settings.Security.VerifyCursorConfiguration = false;
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

    if (!validateVersionRestrictions(sessionConfiguration_, error) ||
        (sessionConfiguration_.settings.SessionMode != seb::settings::SessionMode::Server &&
         (!validateSecuritySettings(sessionConfiguration_.settings, error) ||
          !seb::browser::applyWebEngineEnvironment(sessionConfiguration_.settings.Browser, error)))) return false;

    if (session_) serverProxy_ = session_->takeServerProxy();
    if (!finalizeServerSession(error)) {
        if (isReconfiguration) shutdown();
        return false;
    }
    if (quitRequestedDuringWorkflow_) {
        if (error) *error = QStringLiteral("Session startup cancelled by shutdown.");
        shutdown();
        return false;
    }
    if (sessionConfiguration_.settings.SessionMode == seb::settings::SessionMode::Server &&
        !initializeServerSession(sessionConfiguration_.settings, error)) {
        if (isReconfiguration || quitRequestedDuringWorkflow_) shutdown();
        return false;
    }
    if (!validateVersionRestrictions(sessionConfiguration_, error) || !validateSecuritySettings(sessionConfiguration_.settings, error)) {
        finalizeServerSession();
        if (isReconfiguration) shutdown();
        return false;
    }

    security_->stopMonitoring();
    const bool applied = applySettings(sessionConfiguration_, warnings, error);
    if (!applied) {
        finalizeServerSession();
        if (isReconfiguration) shutdown();
    }
    return applied;
}

bool AppController::applySettings(
    const seb::configuration::contracts::SessionConfiguration &configuration,
    const QStringList &warnings, QString *error)
{
    if (!seb::browser::applyWebEngineEnvironment(configuration.settings.Browser, error)) return false;
    if (!session_) return commitSettings(configuration, warnings, error);
    if (!sessionIntegrityStore_->prepare(
            configuration.settings.Browser.ConfigurationKey,
            !configuration.settings.Security.QuitPasswordHash.isEmpty(),
            configuration.settings.Security.VerifySessionIntegrity,
            error)) {
        if (error && !error->isEmpty())
            QMessageBox::critical(mainWindow_, QStringLiteral("Session integrity check failed"), *error);
        return false;
    }
    transitionPending_ = true;
    QTimer::singleShot(0, this, [this, configuration, warnings] {
        if (shuttingDown_) return;
        const auto windows = session_->browserWindows();
        for (auto *window : windows) delete window;
        mainWindow_ = nullptr;
        const auto &old = session_->configuration();
        const bool discardProfile = ephemeralBrowserProfile_ && !old.isBrowserResource &&
            old.settings.Browser.DeleteCacheOnShutdown && old.settings.Browser.DeleteCookiesOnShutdown;
        session_->finalizeBrowser([this, configuration, warnings, discardProfile](bool finalized) {
            QTimer::singleShot(0, this, [this, configuration, warnings, discardProfile, finalized] {
                if (shuttingDown_) return;
                session_.reset();
                if (discardProfile) {
                    browserProfile_.reset();
                    ephemeralBrowserProfile_ = false;
                }
                QString error;
                const bool committed = finalized && commitSettings(configuration, warnings, &error);
                transitionPending_ = false;
                if (!committed) {
                    if (error.isEmpty()) error = QStringLiteral("Browser data cleanup could not be confirmed. Restart Safe Exam Browser before starting another exam.");
                    QMessageBox::critical(nullptr, QStringLiteral("Session replacement failed"), error);
                    shutdown();
                }
            });
        }, discardProfile);
    });
    return true;
}

bool AppController::commitSettings(
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

    if (!settings.Service.IgnoreService &&
        settings.Service.Policy == seb::settings::service::ServicePolicy::Warn) {
        QMessageBox::warning(mainWindow_, QStringLiteral("SEB service unavailable"),
            QStringLiteral("This configuration requests a service warning. A privileged SEB service is unavailable on Linux; its system controls cannot be applied."));
    }
    QTextStream err(stderr);
    for (const QString &warning : warnings) {
        err << "warning: " << warning << Qt::endl;
    }

    if (!browserProfile_) {
        ephemeralBrowserProfile_ = configuration.isBrowserResource ||
            (settings.Browser.DeleteCookiesOnStartup && settings.Browser.DeleteCookiesOnShutdown &&
             settings.Browser.DeleteCacheOnShutdown);
    }
    if (!ensureBrowserProfileContext(settings.Browser, error)) return false;
    const bool hasQuitPassword = !settings.Security.QuitPasswordHash.isEmpty();
    if (!sessionIntegrityStore_->prepare(
            settings.Browser.ConfigurationKey,
            hasQuitPassword,
            settings.Security.VerifySessionIntegrity,
            error)) {
        if (error && !error->isEmpty())
            QMessageBox::critical(mainWindow_, QStringLiteral("Session integrity check failed"), *error);
        return false;
    }
    if (!security_->configure(settings, error)) return false;
    if (!sessionIntegrityStore_->activate(
            settings.Browser.ConfigurationKey,
            hasQuitPassword,
            settings.Security.VerifySessionIntegrity,
            error)) {
        if (error && !error->isEmpty())
            QMessageBox::critical(mainWindow_, QStringLiteral("Session integrity check failed"), *error);
        return false;
    }

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

    auto nextSession = std::make_unique<SebSession>(
        configuration,
        [this](const QString &resource, QWidget *parent) {
            QString error;
            if (launch(resource, &error)) {
                return true;
            }

            if (!shuttingDown_) QMessageBox::critical(parent, QStringLiteral("Failed to Open SEB Resource"), error);
            return false;
        },
        this,
        browserEngineProvider_.get(),
        browserProfile_.get(),
        std::move(serverProxy_));

    connect(nextSession.get(), &SebSession::shutdownRequested, this, &AppController::shutdown, Qt::QueuedConnection);
    BrowserWindow *nextWindow = nextSession->createWindow(nextSession->initialUrl(), true);
    runtimeWindow_->setStatusText(QStringLiteral("Opening secure browser window"));

    nextWindow->show();
    runtimeWindow_->hide();

    mainWindow_ = nextWindow;
    session_ = std::move(nextSession);
    session_->startRuntimeMonitoring();
    security_->startMonitoring();
    return true;
}
