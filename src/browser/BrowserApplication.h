#pragma once

#include "BrowserApplicationContext.h"
#include "contracts/IBrowserApplication.h"

#include <QObject>

class SebSession;

namespace seb::browser {

class BrowserApplication : public QObject, public contracts::IBrowserApplication
{
    Q_OBJECT

public:
    explicit BrowserApplication(SebSession &session, QObject *parent = nullptr);

    bool autoStart() const override;
    QString iconPath() const override;
    QString id() const override;
    QString name() const override;
    QString tooltip() const override;
    QList<seb::applications::contracts::IApplicationWindow *> windows() const override;
    QList<contracts::IBrowserWindow *> browserWindows() const override;
    void focus(bool forward) override;
    void initialize() override;
    void start() override;
    void terminate() override;

private:
    SebSession &session_;
    BrowserApplicationContext context_;
};

}  // namespace seb::browser
