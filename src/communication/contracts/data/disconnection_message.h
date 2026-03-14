#pragma once

#include "../interlocutor.h"
#include "message.h"

namespace seb::communication::contracts::data {

struct DisconnectionMessage : Message
{
    seb::communication::contracts::Interlocutor interlocutor = seb::communication::contracts::Interlocutor::Unknown;
};

}  // namespace seb::communication::contracts::data
