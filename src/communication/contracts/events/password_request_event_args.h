#pragma once

#include "../data/password_request_purpose.h"
#include "communication_event_args.h"

#include <QUuid>

namespace seb::communication::contracts::events {

struct PasswordRequestEventArgs : CommunicationEventArgs
{
    seb::communication::contracts::data::PasswordRequestPurpose purpose =
        seb::communication::contracts::data::PasswordRequestPurpose::Settings;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::events
