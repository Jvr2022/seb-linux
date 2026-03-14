#pragma once

#include "communication_event_args.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::events {

struct PasswordReplyEventArgs : CommunicationEventArgs
{
    QString password;
    QUuid requestId;
    bool success = false;
};

}  // namespace seb::communication::contracts::events
