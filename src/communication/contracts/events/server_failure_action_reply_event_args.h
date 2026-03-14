#pragma once

#include "communication_event_args.h"

#include <QUuid>

namespace seb::communication::contracts::events {

struct ServerFailureActionReplyEventArgs : CommunicationEventArgs
{
    bool abort = false;
    bool fallback = false;
    bool retry = false;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::events
