#pragma once

#include "message.h"

#include "../../../seb_settings.h"

namespace seb::communication::contracts::data {

struct SessionStartMessage : Message
{
    explicit SessionStartMessage(seb::SebSettings value = {})
        : configuration(std::move(value)) {}

    seb::SebSettings configuration;
};

}  // namespace seb::communication::contracts::data
