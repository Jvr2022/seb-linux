#pragma once

#include "message.h"

#include "../../../settings/AppSettings.h"

namespace seb::communication::contracts::data {

struct SessionStartMessage : Message
{
    explicit SessionStartMessage(seb::settings::AppSettings value = {})
        : configuration(std::move(value)) {}

    seb::settings::AppSettings configuration;
};

}  // namespace seb::communication::contracts::data
