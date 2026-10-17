#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>

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

private slots:
    void onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties);

private:
    BatteryState state_;
};

}  // namespace seb::shell::taskbar::platform
