#pragma once

#include <QObject>
#include <QList>
#include <QString>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace seb::shell::taskbar::platform {

struct WirelessNetwork
{
    QString ssid;
    int signalPercent = 0;
    bool active = false;
};

inline bool operator==(const WirelessNetwork &lhs, const WirelessNetwork &rhs)
{
    return lhs.ssid == rhs.ssid && lhs.signalPercent == rhs.signalPercent && lhs.active == rhs.active;
}

struct NetworkState
{
    enum class Type { None, Wired, Wireless };
    enum class Status { Disconnected, Connected, Connecting };

    bool available = false;
    Type type = Type::None;
    Status status = Status::Disconnected;
    QString activeConnection;
    QList<WirelessNetwork> networks;
};

class NetworkController : public QObject
{
    Q_OBJECT

public:
    explicit NetworkController(QObject *parent = nullptr);

    const NetworkState &state() const;
    void refresh();

signals:
    void stateChanged();

private:
    NetworkState state_;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
