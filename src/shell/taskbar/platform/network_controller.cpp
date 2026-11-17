#include "network_controller.h"

#include "command_helper.h"

#include <QTimer>
#include <QProcess>

namespace seb::shell::taskbar::platform {

namespace {
QStringList fields(const QString &line)
{
    QStringList result;
    QString field;
    bool escaped = false;
    for (const QChar character : line) {
        if (escaped) { field += character; escaped = false; }
        else if (character == QLatin1Char('\\')) escaped = true;
        else if (character == QLatin1Char(':')) { result.append(field); field.clear(); }
        else field += character;
    }
    if (escaped) field += QLatin1Char('\\');
    result.append(field);
    return result;
}
}

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

void NetworkController::stopMonitoring()
{
    timer_->stop();
    ++generation_;
    refreshInFlight_ = false;
    for (auto *process : findChildren<QProcess *>(QString(), Qt::FindDirectChildrenOnly)) process->kill();
}

void NetworkController::refresh()
{
    if (refreshInFlight_ || !timer_->isActive()) return;
    refreshInFlight_ = true;
    const auto generation = generation_;
    const auto publish = [this, generation](NetworkState next) {
        if (generation != generation_) return;
        refreshInFlight_ = false;
        const bool changed = next.available != state_.available || next.type != state_.type ||
            next.status != state_.status || next.activeConnection != state_.activeConnection || next.networks != state_.networks;
        if (changed) { state_ = std::move(next); emit stateChanged(); }
    };
    runCommandAsync(this, QStringLiteral("nmcli"),
        {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("DEVICE,TYPE,STATE,CONNECTION"), QStringLiteral("device")},
        [this, generation, publish](const QString &output) {
            if (generation != generation_) return;
            NetworkState next;
            int selectedRank = -1;
            for (const auto &line : output.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
                const auto parts = fields(line);
                if (parts.size() != 4) continue;
                const auto type = parts.at(1);
                if (type != QStringLiteral("wifi") && type != QStringLiteral("ethernet")) continue;
                const auto status = parts.at(2).startsWith(QStringLiteral("connected")) ? NetworkState::Status::Connected :
                    parts.at(2).contains(QStringLiteral("connecting")) ? NetworkState::Status::Connecting : NetworkState::Status::Disconnected;
                const int rank = (status == NetworkState::Status::Connected ? 10 : status == NetworkState::Status::Connecting ? 5 : 0) +
                    (type == QStringLiteral("wifi") ? 2 : 1);
                if (rank <= selectedRank) continue;
                selectedRank = rank;
                next.available = true;
                next.type = type == QStringLiteral("wifi") ? NetworkState::Type::Wireless : NetworkState::Type::Wired;
                next.activeConnection = parts.at(3);
                next.status = status;
            }
            if (next.type != NetworkState::Type::Wireless) { publish(next); return; }
            runCommandAsync(this, QStringLiteral("nmcli"),
                {QStringLiteral("-t"), QStringLiteral("-f"), QStringLiteral("ACTIVE,SSID,SIGNAL"),
                 QStringLiteral("device"), QStringLiteral("wifi"), QStringLiteral("list"), QStringLiteral("--rescan"), QStringLiteral("no")},
                [this, generation, next, publish](const QString &wireless) mutable {
                    if (generation != generation_) return;
                    for (const auto &line : wireless.split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
                        const auto parts = fields(line);
                        if (parts.size() != 3 || parts.at(1).isEmpty()) continue;
                        bool valid = false;
                        const int signal = parts.at(2).toInt(&valid);
                        if (!valid || signal < 0 || signal > 100) continue;
                        next.networks.append({parts.at(1), signal, parts.at(0) == QStringLiteral("yes")});
                    }
                    publish(next);
                });
        });
}

}
