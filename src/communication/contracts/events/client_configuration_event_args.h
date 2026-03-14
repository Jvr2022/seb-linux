#pragma once

#include "communication_event_args.h"

#include "../../../seb_settings.h"

namespace seb::communication::contracts::events {

struct ClientConfigurationEventArgs : CommunicationEventArgs
{
    seb::SebSettings clientConfiguration;
};

}  // namespace seb::communication::contracts::events
