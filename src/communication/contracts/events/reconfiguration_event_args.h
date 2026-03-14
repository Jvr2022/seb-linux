#pragma once

#include "communication_event_args.h"

#include <QString>

namespace seb::communication::contracts::events {

struct ReconfigurationEventArgs : CommunicationEventArgs
{
    QString configurationPath;
    QString resourceUrl;
};

}  // namespace seb::communication::contracts::events
