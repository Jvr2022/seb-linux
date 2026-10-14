#pragma once

#include "ServicePolicy.h"

namespace seb::settings::service {

struct ServiceSettings
{
    bool DisableUserLock = false;
    bool DisableUserSwitch = false;
    bool IgnoreService = true;
    ServicePolicy Policy = ServicePolicy::Ignore;
    bool SetVmwareConfiguration = false;
    bool DisableChromeNotifications = true;
    bool DisableEaseOfAccessOptions = true;
    bool DisableFindPrinter = true;
    bool DisableNetworkOptions = true;
    bool DisablePasswordChange = true;
    bool DisablePowerOptions = true;
    bool DisableRemoteConnections = true;
    bool DisableSignout = true;
    bool DisableTaskManager = true;
    bool DisableVmwareOverlay = true;
    bool DisableWindowsUpdate = true;
};

}  // namespace seb::settings::service
