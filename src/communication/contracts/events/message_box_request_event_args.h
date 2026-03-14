#pragma once

#include "communication_event_args.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::events {

struct MessageBoxRequestEventArgs : CommunicationEventArgs
{
    int action = 0;
    int icon = 0;
    QString message;
    QUuid requestId;
    QString title;
};

}  // namespace seb::communication::contracts::events
