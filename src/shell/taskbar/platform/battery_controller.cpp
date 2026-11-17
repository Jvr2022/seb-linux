#include "battery_controller.h"

#include "command_helper.h"

#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QProcess>

namespace seb::shell::taskbar::platform {

BatteryController::BatteryController(QObject *parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(10000);
    connect(timer_, &QTimer::timeout, this, &BatteryController::refresh);
    timer_->start();
    refresh();
}

const BatteryState &BatteryController::state() const
{
    return state_;
}

void BatteryController::stopMonitoring()
{
    timer_->stop();
    ++generation_;
    refreshInFlight_ = false;
    for (auto *process : findChildren<QProcess *>(QString(), Qt::FindDirectChildrenOnly)) process->kill();
}

void BatteryController::refresh()
{
    if (refreshInFlight_ || !timer_->isActive()) return;
    refreshInFlight_ = true;
    const auto generation = generation_;
    runCommandAsync(this, QStringLiteral("upower"),
        {QStringLiteral("-i"), QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice")},
        [this, generation](const QString &output) {
            if (generation != generation_) return;
            refreshInFlight_ = false;
            applySnapshot(output);
        });
}

void BatteryController::applySnapshot(const QString &output)
{
    BatteryState next;
    if (!output.isEmpty()) {
        bool absent = false;
        const QStringList lines = output.split('\n');
        for (const QString &line : lines) {
            const QString trimmed = line.trimmed();
            if (trimmed.startsWith(QStringLiteral("percentage:"))) {
                const QRegularExpression re(QStringLiteral("([0-9]+(?:\\.[0-9]+)?)%"));
                const auto match = re.match(trimmed);
                if (match.hasMatch()) {
                    const double percentage = match.captured(1).toDouble();
                    if (percentage >= 0 && percentage <= 100) {
                        next.percentage = qRound(percentage);
                        next.available = true;
                    }
                }
            } else if (trimmed.startsWith(QStringLiteral("present:"))) {
                absent = trimmed.section(':', 1).trimmed() == QStringLiteral("no");
            } else if (trimmed.startsWith(QStringLiteral("state:"))) {
                const QString value = trimmed.section(':', 1).trimmed();
                next.charging = value == QStringLiteral("charging") || value == QStringLiteral("pending-charge");
                next.powerConnected = next.charging || value == QStringLiteral("fully-charged");
            } else if (trimmed.startsWith(QStringLiteral("time to empty:")) || trimmed.startsWith(QStringLiteral("time to full:"))) {
                next.timeRemaining = trimmed.section(':', 1).trimmed();
            }
        }
        if (absent) next.available = false;
    }

    bool foundSupply = false;
    bool online = false;
    const QDir supplies(QStringLiteral("/sys/class/power_supply"));
    for (const auto &supply : supplies.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QFile file(supplies.filePath(supply + QStringLiteral("/online")));
        if (file.open(QIODevice::ReadOnly)) {
            foundSupply = true;
            online |= file.readAll().trimmed() == QByteArrayLiteral("1");
        }
    }
    if (foundSupply) next.powerConnected = online;

    if (next.available != state_.available ||
        next.powerConnected != state_.powerConnected ||
        next.charging != state_.charging ||
        next.percentage != state_.percentage ||
        next.timeRemaining != state_.timeRemaining) {
        state_ = next;
        emit stateChanged();
    }
}

}  // namespace seb::shell::taskbar::platform
