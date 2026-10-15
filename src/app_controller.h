#pragma once

#include "configuration/configuration_repository.h"
#include "configuration/contracts/session_configuration.h"

#include <memory>

#include <QObject>

class BrowserWindow;
class RuntimeWindow;
class SebSession;
class QWidget;

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController() override;

    bool launch(const QString &resource, QString *error);

private:
    bool applySettings(
        const seb::configuration::contracts::SessionConfiguration &configuration,
        const QStringList &warnings,
        QString *error);

    seb::configuration::ConfigurationRepository repository_;
    seb::configuration::contracts::SessionConfiguration sessionConfiguration_;
    std::unique_ptr<RuntimeWindow> runtimeWindow_;
    std::unique_ptr<SebSession> session_;
    BrowserWindow *mainWindow_ = nullptr;
};
