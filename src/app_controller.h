#pragma once

#include "configuration/configuration_repository.h"
#include "configuration/contracts/session_configuration.h"

#include <memory>

#include <QObject>

class BrowserWindow;
class RuntimeWindow;
class SebSession;
class QTemporaryDir;
class QWidget;

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

    bool launch(const QString &resource, QString *error);

private:
    bool loadSettingsWithPassword(
        const QUrl &resource,
        seb::settings::AppSettings &settings,
        seb::configuration::contracts::cryptography::PasswordParameters &password,
        const QString &currentPasswordHash,
        bool localSettings,
        seb::configuration::contracts::LoadStatus &status,
        bool &aborted);

    bool ensureBrowserProfile();
    bool ensureBrowserProfileContext();

    bool initializeServerSession(
        seb::settings::AppSettings &settings,
        QString *error);

    bool applySettings(
        const seb::configuration::contracts::SessionConfiguration &configuration,
        const QStringList &warnings,
        QString *error);

    seb::configuration::ConfigurationRepository repository_;
    seb::configuration::contracts::SessionConfiguration sessionConfiguration_;
    std::unique_ptr<RuntimeWindow> runtimeWindow_;
    std::unique_ptr<QTemporaryDir> browserProfileDirectory_;
    std::unique_ptr<seb::browser::contracts::IEngineProvider> browserEngineProvider_;
    std::unique_ptr<seb::browser::contracts::IWebProfile> browserProfile_;
    std::unique_ptr<SebSession> session_;
    std::unique_ptr<seb::server::ServerProxy> serverProxy_;
    BrowserWindow *mainWindow_ = nullptr;
    bool reconfigurationInProgress_ = false;
};
