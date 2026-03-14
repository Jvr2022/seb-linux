#pragma once

#include "message.h"
#include "simple_message_purport.h"

namespace seb::communication::contracts::data {

struct SimpleMessage : Message
{
    explicit SimpleMessage(SimpleMessagePurport value)
        : purport(value) {}

    SimpleMessagePurport purport;
};

}  // namespace seb::communication::contracts::data
