#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>
#include <QTimerEvent>
#include "settings/AppSettings.h"

namespace seb::security {

class SecurityService : public QObject
{
    Q_OBJECT

public:
    explicit SecurityService(QObject *parent = nullptr);

    bool validateSettings(const seb::settings::AppSettings &settings, QString *error = nullptr) const;
    bool configure(const seb::settings::AppSettings &settings, QString *error = nullptr);
    bool isVirtualMachine() const;
    bool isDebuggerAttached() const;

    void startMonitoring();
    void stopMonitoring();

protected:
    void timerEvent(QTimerEvent *event) override;

signals:
    void secureViolationDetected(const QString &reason);

private:
    void performCheck();
    bool validateDisplay(const seb::settings::monitoring::DisplaySettings &settings, QString *error) const;
    QStringList detectProhibitedProcesses(const seb::settings::applications::ApplicationSettings &settings) const;
    seb::settings::monitoring::DisplaySettings display_;
    seb::settings::security::SecuritySettings security_;
    seb::settings::applications::ApplicationSettings applications_;
    bool alwaysOnRequired_ = false;
    bool violationReported_ = false;
    int timerId_ = -1;
    QProcess idleInhibitor_;
};

} // namespace seb::security
