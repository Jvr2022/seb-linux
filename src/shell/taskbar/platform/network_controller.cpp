#include "network_controller.h"

#include "command_helper.h"

#include <QTimer>

namespace seb::shell::taskbar::platform {

NetworkController::NetworkController(QObject *parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(5000);
    connect(timer_, &QTimer::timeout, this, &NetworkController::refresh);
    timer_->start();
    refresh();
}

const NetworkState &NetworkController::state() const
{
    return state_;
}

void NetworkController::refresh()
{
    NetworkState next;
    const QString deviceOutput = runCommand(
        QStringLiteral("nmcli"),
        {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("DEVICE,TYPE,STATE,CONNECTION"), QStringLiteral("device")});

    const QStringList deviceLines = deviceOutput.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : deviceLines) {
        const QStringList parts = line.split(':');
        if (parts.size() < 4) {
            continue;
        }
        const QString type = parts.at(1);
        const QString state = parts.at(2);
        const QString connection = parts.mid(3).join(QStringLiteral(":"));

        if (type == QStringLiteral("wifi")) {
            next.available = true;
            next.type = NetworkState::Type::Wireless;
            next.activeConnection = connection;
            if (state == QStringLiteral("connected")) {
                next.status = NetworkState::Status::Connected;
            } else if (state.contains(QStringLiteral("connecting"))) {
                next.status = NetworkState::Status::Connecting;
            }
            break;
        }
        if (type == QStringLiteral("ethernet") && next.type == NetworkState::Type::None) {
            next.available = true;
            next.type = NetworkState::Type::Wired;
            next.activeConnection = connection;
            next.status = state == QStringLiteral("connected") ? NetworkState::Status::Connected : NetworkState::Status::Disconnected;
        }
    }

    if (next.type == NetworkState::Type::Wireless) {
        const QString wifiOutput = runCommand(
            QStringLiteral("nmcli"),
            {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("ACTIVE,SSID,SIGNAL"), QStringLiteral("device"), QStringLiteral("wifi"), QStringLiteral("list")});
        const QStringList wifiLines = wifiOutput.split('\n', Qt::SkipEmptyParts);
        for (const QString &line : wifiLines) {
            const QStringList parts = line.split(':');
            if (parts.size() < 3) {
                continue;
            }
            WirelessNetwork network;
            network.active = parts.at(0) == QStringLiteral("yes");
            network.ssid = parts.at(1);
            network.signalPercent = parts.at(2).toInt();
            if (!network.ssid.isEmpty()) {
                next.networks.push_back(network);
            }
        }
    }

    const bool changed =
        next.available != state_.available ||
        next.type != state_.type ||
        next.status != state_.status ||
        next.activeConnection != state_.activeConnection ||
        next.networks != state_.networks;

    if (changed) {
        state_ = next;
        emit stateChanged();
    }
}

}  // namespace seb::shell::taskbar::platform
