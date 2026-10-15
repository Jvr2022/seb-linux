#pragma once

#include "communication_event_args.h"

#include "../../../settings/AppSettings.h"

namespace seb::communication::contracts::events {

struct SessionStartEventArgs : CommunicationEventArgs
{
    seb::settings::AppSettings configuration;
};

}  // namespace seb::communication::contracts::events
