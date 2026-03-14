#pragma once

#include "communication_event_args.h"

#include <QUuid>

namespace seb::communication::contracts::events {

struct SessionStopEventArgs : CommunicationEventArgs
{
    QUuid sessionId;
};

}  // namespace seb::communication::contracts::events
