#pragma once

#include <QObject>
#include <QStringList>
#include <QTimerEvent>

namespace seb::security {

class SecurityService : public QObject
{
    Q_OBJECT

public:
    explicit SecurityService(QObject *parent = nullptr);

    bool isVirtualMachine() const;
    bool isDebuggerAttached() const;
    bool isMultipleDisplaysActive() const;
    QStringList detectProhibitedProcesses() const;
    
    void startMonitoring();
    void stopMonitoring();

protected:
    void timerEvent(QTimerEvent *event) override;

signals:
    void secureViolationDetected(const QString &reason);

private:
    void performCheck();
    int timerId_ = -1;
};

} // namespace seb::security
