#pragma once

#include "response.h"

#include "../../../settings/AppSettings.h"

namespace seb::communication::contracts::data {

struct ConfigurationResponse : Response
{
    seb::settings::AppSettings configuration;
};

}  // namespace seb::communication::contracts::data
