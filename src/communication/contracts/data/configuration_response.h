#pragma once

#include "response.h"

#include "../../../seb_settings.h"

namespace seb::communication::contracts::data {

struct ConfigurationResponse : Response
{
    seb::SebSettings configuration;
};

}  // namespace seb::communication::contracts::data
