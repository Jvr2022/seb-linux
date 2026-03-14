#pragma once

#include "communication_event_args.h"

#include "../../../seb_settings.h"

namespace seb::communication::contracts::events {

struct SessionStartEventArgs : CommunicationEventArgs
{
    seb::SebSettings configuration;
};

}  // namespace seb::communication::contracts::events
