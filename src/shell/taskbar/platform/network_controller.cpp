#include "network_controller.h"

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
    if ((deviceProcess_ && deviceProcess_->state() != QProcess::NotRunning) ||
        (wifiProcess_ && wifiProcess_->state() != QProcess::NotRunning)) {
        return;
    }

    pendingState_ = NetworkState();

    if (!deviceProcess_) {
        deviceProcess_ = new QProcess(this);
        connect(deviceProcess_, &QProcess::finished, this, &NetworkController::onDeviceQueryFinished);
    }

    deviceProcess_->start(
        QStringLiteral("nmcli"),
        {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("DEVICE,TYPE,STATE,CONNECTION"), QStringLiteral("device")}
    );
}

void NetworkController::onDeviceQueryFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        pendingState_.available = false;
        finalizeState();
        return;
    }

    const QString deviceOutput = QString::fromUtf8(deviceProcess_->readAllStandardOutput()).trimmed();
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
            pendingState_.available = true;
            pendingState_.type = NetworkState::Type::Wireless;
            pendingState_.activeConnection = connection;
            if (state == QStringLiteral("connected")) {
                pendingState_.status = NetworkState::Status::Connected;
            } else if (state.contains(QStringLiteral("connecting"))) {
                pendingState_.status = NetworkState::Status::Connecting;
            }
            break;
        }
        if (type == QStringLiteral("ethernet") && pendingState_.type == NetworkState::Type::None) {
            pendingState_.available = true;
            pendingState_.type = NetworkState::Type::Wired;
            pendingState_.activeConnection = connection;
            pendingState_.status = state == QStringLiteral("connected") ? NetworkState::Status::Connected : NetworkState::Status::Disconnected;
        }
    }

    if (pendingState_.type == NetworkState::Type::Wireless) {
        if (!wifiProcess_) {
            wifiProcess_ = new QProcess(this);
            connect(wifiProcess_, &QProcess::finished, this, &NetworkController::onWifiListQueryFinished);
        }
        wifiProcess_->start(
            QStringLiteral("nmcli"),
            {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("ACTIVE,SSID,SIGNAL"), QStringLiteral("device"), QStringLiteral("wifi"), QStringLiteral("list")}
        );
    } else {
        finalizeState();
    }
}

void NetworkController::onWifiListQueryFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        const QString wifiOutput = QString::fromUtf8(wifiProcess_->readAllStandardOutput()).trimmed();
        const QStringList wifiLines = wifiOutput.split('\n', Qt::SkipEmptyParts);
        QStringList seenSsids;
        for (const QString &line : wifiLines) {
            const QStringList parts = line.split(':');
            if (parts.size() < 3) {
                continue;
            }
            WirelessNetwork network;
            network.active = parts.at(0) == QStringLiteral("yes");
            network.ssid = parts.at(1);
            network.signalPercent = parts.at(2).toInt();
            
            if (network.ssid.isEmpty()) {
                continue;
            }

            int index = seenSsids.indexOf(network.ssid);
            if (index != -1) {
                WirelessNetwork &existing = pendingState_.networks[index];
                if (network.active) {
                    existing.active = true;
                }
                if (network.signalPercent > existing.signalPercent) {
                    existing.signalPercent = network.signalPercent;
                }
            } else {
                seenSsids.append(network.ssid);
                pendingState_.networks.push_back(network);
            }
        }
    }

    finalizeState();
}

void NetworkController::finalizeState()
{
    const bool changed =
        pendingState_.available != state_.available ||
        pendingState_.type != state_.type ||
        pendingState_.status != state_.status ||
        pendingState_.activeConnection != state_.activeConnection ||
        pendingState_.networks != state_.networks;

    if (changed) {
        state_ = pendingState_;
        emit stateChanged();
    }
}

void NetworkController::connectToNetwork(const QString &ssid)
{
    if (ssid.isEmpty()) {
        return;
    }

    QProcess::startDetached(QStringLiteral("nmcli"), {QStringLiteral("device"), QStringLiteral("wifi"), QStringLiteral("connect"), ssid});

    if (state_.activeConnection != ssid || state_.status != NetworkState::Status::Connecting) {
        state_.activeConnection = ssid;
        state_.status = NetworkState::Status::Connecting;
        emit stateChanged();
    }

    QTimer::singleShot(2000, this, &NetworkController::refresh);
}

}  // namespace seb::shell::taskbar::platform
