#include "battery_controller.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

namespace seb::shell::taskbar::platform {

BatteryController::BatteryController(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().connect(
        QStringLiteral("org.freedesktop.UPower"),
        QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice"),
        QStringLiteral("org.freedesktop.DBus.Properties"),
        QStringLiteral("PropertiesChanged"),
        this,
        SLOT(onPropertiesChanged(QString, QVariantMap, QStringList))
    );

    refresh();
}

const BatteryState &BatteryController::state() const
{
    return state_;
}

void BatteryController::refresh()
{
    QDBusInterface iface(
        QStringLiteral("org.freedesktop.UPower"),
        QStringLiteral("/org/freedesktop/UPower/devices/DisplayDevice"),
        QStringLiteral("org.freedesktop.UPower.Device"),
        QDBusConnection::systemBus()
    );

    if (!iface.isValid()) {
        BatteryState next;
        next.available = false;
        if (next.available != state_.available) {
            state_ = next;
            emit stateChanged();
        }
        return;
    }

    BatteryState next;
    next.available = true;

    double percentageVal = iface.property("Percentage").toDouble();
    uint stateVal = iface.property("State").toUInt();
    qlonglong timeToFull = iface.property("TimeToFull").toLongLong();
    qlonglong timeToEmpty = iface.property("TimeToEmpty").toLongLong();

    next.percentage = qBound(0, qRound(percentageVal), 100);
    next.charging = (stateVal == 1); // 1 = Charging in UPower
    
    if (stateVal == 0) {
        next.available = false;
    }

    qlonglong seconds = 0;
    if (stateVal == 1) {
        seconds = timeToFull;
    } else if (stateVal == 2) { // 2 = Discharging
        seconds = timeToEmpty;
    }

    // Limit to < 24h to ignore UPower calculation anomalies
    if (seconds > 0 && seconds < 86400) {
        int hours = seconds / 3600;
        int minutes = (seconds % 3600) / 60;
        if (hours > 0) {
            if (minutes > 0) {
                next.timeRemaining = QStringLiteral("%1h %2m").arg(hours).arg(minutes);
            } else {
                next.timeRemaining = QStringLiteral("%1h").arg(hours);
            }
        } else if (minutes > 0) {
            next.timeRemaining = QStringLiteral("%1m").arg(minutes);
        }
    }

    if (next.available != state_.available ||
        next.charging != state_.charging ||
        next.percentage != state_.percentage ||
        next.timeRemaining != state_.timeRemaining) {
        state_ = next;
        emit stateChanged();
    }
}

void BatteryController::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(interfaceName);
    Q_UNUSED(changedProperties);
    Q_UNUSED(invalidatedProperties);
    refresh();
}

}  // namespace seb::shell::taskbar::platform
