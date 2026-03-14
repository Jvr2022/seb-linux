#pragma once

#include "communication_event_args.h"

#include <QUuid>

namespace seb::communication::contracts::events {

struct MessageBoxReplyEventArgs : CommunicationEventArgs
{
    QUuid requestId;
    int result = 0;
};

}  // namespace seb::communication::contracts::events
