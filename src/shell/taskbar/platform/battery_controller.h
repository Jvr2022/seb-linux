#pragma once

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace seb::shell::taskbar::platform {

struct BatteryState
{
    bool available = false;
    bool charging = false;
    bool powerConnected = false;
    int percentage = 0;
    QString timeRemaining;
};

class BatteryController : public QObject
{
    Q_OBJECT

public:
    explicit BatteryController(QObject *parent = nullptr);

    const BatteryState &state() const;
    void refresh();
    void stopMonitoring();

signals:
    void stateChanged();

private:
    void applySnapshot(const QString &output);
    BatteryState state_;
    QTimer *timer_ = nullptr;
    bool refreshInFlight_ = false;
    quint64 generation_ = 0;
};

}  // namespace seb::shell::taskbar::platform
