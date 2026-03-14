#include "verificator_notification.h"

namespace seb::client::notifications {

VerificatorNotification::VerificatorNotification(QObject *parent)
    : QObject(parent)
{
}

QString VerificatorNotification::iconPath() const
{
    return QStringLiteral(":/assets/taskbar/verificator.svg");
}

QString VerificatorNotification::tooltip() const
{
    return QStringLiteral("Verificator");
}

void VerificatorNotification::activate()
{
    emit activationRequested();
}

void VerificatorNotification::terminate()
{
    emit terminationRequested();
}

}  // namespace seb::client::notifications
