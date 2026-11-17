#pragma once

#include "configuration/configuration_repository.h"
#include "configuration/contracts/session_configuration.h"

#include <memory>
#include <functional>

#include <QObject>

class BrowserWindow;
class RuntimeWindow;
class SebSession;
class QWidget;

namespace seb::security {
class SecurityService;
class SessionIntegrityStore;
}

namespace seb::server {
class ServerProxy;
}

namespace seb::browser::contracts {
class IEngineProvider;
class IWebProfile;
}

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    void shutdown();
    bool launch(const QString &resource, QString *error);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    bool loadSettingsWithPassword(
        const QUrl &resource,
        seb::settings::AppSettings &settings,
        seb::configuration::contracts::cryptography::PasswordParameters &password,
        const QString &currentPasswordHash,
        bool localSettings,
        seb::configuration::contracts::LoadStatus &status,
        bool &aborted);

    bool ensureBrowserProfileContext(const seb::settings::browser::BrowserSettings &settings, QString *error);

    void completeShutdown();
    bool validateSecuritySettings(const seb::settings::AppSettings &settings, QString *error);
    bool finalizeServerSession(QString *error = nullptr);
    bool initializeServerSession(
        seb::settings::AppSettings &settings,
        QString *error);

    bool commitSettings(
        const seb::configuration::contracts::SessionConfiguration &configuration,
        const QStringList &warnings, QString *error);
    bool applySettings(
        const seb::configuration::contracts::SessionConfiguration &configuration,
        const QStringList &warnings,
        QString *error);

    seb::configuration::ConfigurationRepository repository_;
    seb::configuration::contracts::SessionConfiguration sessionConfiguration_;
    std::unique_ptr<RuntimeWindow> runtimeWindow_;
    bool ephemeralBrowserProfile_ = false;
    std::unique_ptr<seb::browser::contracts::IEngineProvider> browserEngineProvider_;
    std::unique_ptr<seb::browser::contracts::IWebProfile> browserProfile_;
    std::unique_ptr<SebSession> session_;
    std::unique_ptr<seb::security::SecurityService> security_;
    std::unique_ptr<seb::security::SessionIntegrityStore> sessionIntegrityStore_;
    std::unique_ptr<seb::server::ServerProxy> serverProxy_;
    BrowserWindow *mainWindow_ = nullptr;
    std::function<void()> abortServerWorkflow_;
    bool serverWorkflowActive_ = false;
    bool quitRequestedDuringWorkflow_ = false;
    bool shuttingDown_ = false;
    bool shutdownComplete_ = false;
    bool browserShutdownFinalizationStarted_ = false;
    bool browserShutdownFinalizationComplete_ = false;
    bool browserShutdownFinalizationSucceeded_ = true;
    bool sessionIntegrityCleanupAllowed_ = true;
    bool reconfigurationInProgress_ = false;
    bool transitionPending_ = false;
};
