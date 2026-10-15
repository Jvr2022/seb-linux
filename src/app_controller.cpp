#include "app_controller.h"

#include "browser_window.h"
#include "browser/webengine_environment.h"
#include "seb_session.h"
#include "ui/runtime_window.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
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

}  // namespace

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    sessionConfiguration_ = repository_.initializeSessionConfiguration();
}

AppController::~AppController() = default;

bool AppController::launch(const QString &resource, QString *error)
{
    sessionConfiguration_ = repository_.initializeSessionConfiguration();
    const QStringList warnings;
    const bool hasRequestedResource = !resource.trimmed().isEmpty();

    QUrl targetResource;
    if (hasRequestedResource) {
        targetResource = resourceUrl(resource);
    } else {
        const QStringList persistentResources = {
            sessionConfiguration_.appConfig.programDataFilePath,
            sessionConfiguration_.appConfig.appDataFilePath,
        };
        for (const QString &path : persistentResources) {
            if (QFileInfo::exists(path)) {
                targetResource = QUrl::fromLocalFile(path);
                break;
            }
        }
    }

    if (!targetResource.isValid() || targetResource.isEmpty()) {
        sessionConfiguration_.settings = repository_.loadDefaultSettings();
    } else {
        auto &settings = sessionConfiguration_.settings;
        seb::configuration::contracts::cryptography::PasswordParameters emptyPassword;
        emptyPassword.password = QString();
        emptyPassword.isHash = true;
        auto passwordUsed = emptyPassword;
        auto status = repository_.tryLoadSettings(targetResource, settings, &emptyPassword);

        if (hasRequestedResource && status == seb::configuration::contracts::LoadStatus::PasswordNeeded) {
            const QStringList persistentResources = {
                sessionConfiguration_.appConfig.programDataFilePath,
                sessionConfiguration_.appConfig.appDataFilePath,
            };
            for (const QString &path : persistentResources) {
                if (!QFileInfo::exists(path)) {
                    continue;
                }
                seb::settings::AppSettings clientSettings;
                const auto clientStatus = repository_.tryLoadSettings(
                    QUrl::fromLocalFile(path),
                    clientSettings,
                    &emptyPassword);
                if (clientStatus == seb::configuration::contracts::LoadStatus::Success &&
                    !clientSettings.Security.AdminPasswordHash.isEmpty()) {
                    passwordUsed.password = clientSettings.Security.AdminPasswordHash;
                    passwordUsed.isHash = true;
                    status = repository_.tryLoadSettings(targetResource, settings, &passwordUsed);
                    break;
                }
            }
        }

        if (status == seb::configuration::contracts::LoadStatus::PasswordNeeded) {
            bool loaded = false;
            for (int attempt = 0; attempt < 5 && !loaded; ++attempt) {
                bool accepted = false;
                const QString password = QInputDialog::getText(
                    mainWindow_,
                    QStringLiteral("SEB Password Required"),
                    QStringLiteral("Enter the password for this SEB configuration file."),
                    QLineEdit::Password,
                    QString(),
                    &accepted);
                if (!accepted) {
                    break;
                }

                seb::configuration::contracts::cryptography::PasswordParameters enteredPassword;
                enteredPassword.password = password;
                enteredPassword.isHash = false;
                passwordUsed = enteredPassword;
                status = repository_.tryLoadSettings(targetResource, settings, &enteredPassword);
                if (status == seb::configuration::contracts::LoadStatus::Success) {
                    loaded = true;
                } else if (status != seb::configuration::contracts::LoadStatus::PasswordNeeded) {
                    break;
                }
            }

            if (!loaded) {
                if (error) *error = QStringLiteral("The SEB configuration password was not accepted.");
                return false;
            }
        } else if (status == seb::configuration::contracts::LoadStatus::LoadWithBrowser) {
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
            settings.UserInterface.ActionCenter.EnableActionCenter = false;
        } else if (status != seb::configuration::contracts::LoadStatus::Success) {
            if (error) {
                *error = loadStatusMessage(status);
            }
            return false;
        }

        if (hasRequestedResource &&
            status == seb::configuration::contracts::LoadStatus::Success &&
            settings.ConfigurationMode == seb::settings::ConfigurationMode::ConfigureClient) {
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
            if (error) *error = QStringLiteral("Client configuration saved successfully.");
            return false;
        }
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
        this);

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
