#pragma once

#include <QObject>
#include <QList>
#include <QString>
#include <QProcess>

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
    void connectToNetwork(const QString &ssid);

signals:
    void stateChanged();

private slots:
    void onDeviceQueryFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onWifiListQueryFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    void finalizeState();

    NetworkState state_;
    NetworkState pendingState_;
    QTimer *timer_ = nullptr;
    QProcess *deviceProcess_ = nullptr;
    QProcess *wifiProcess_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
