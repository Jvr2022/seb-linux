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

signals:
    void stateChanged();

private:
    BatteryState state_;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
