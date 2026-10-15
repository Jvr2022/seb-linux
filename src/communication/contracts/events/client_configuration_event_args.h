#pragma once

#include "communication_event_args.h"

#include "../../../settings/AppSettings.h"

namespace seb::communication::contracts::events {

struct ClientConfigurationEventArgs : CommunicationEventArgs
{
    seb::settings::AppSettings clientConfiguration;
};

}  // namespace seb::communication::contracts::events
