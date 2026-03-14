#pragma once

#include "ServicePolicy.h"

namespace seb::settings::service {

struct ServiceSettings
{
    bool DisableUserLock = false;
    bool DisableUserSwitch = false;
    bool IgnoreService = true;
    ServicePolicy Policy = ServicePolicy::Ignore;
};

}  // namespace seb::settings::service
