#pragma once

#include "seb_settings.h"

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
    bool launchResolved(const seb::SebSettings &settings, const QStringList &warnings, QString *error);

private:
    bool applySettings(const seb::SebSettings &settings, const QStringList &warnings, QString *error);

    std::unique_ptr<RuntimeWindow> runtimeWindow_;
    std::unique_ptr<SebSession> session_;
    BrowserWindow *mainWindow_ = nullptr;
};
