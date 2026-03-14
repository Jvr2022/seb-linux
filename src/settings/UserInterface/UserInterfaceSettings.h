#pragma once

#include "ActionCenterSettings.h"
#include "LockScreenSettings.h"
#include "TaskbarSettings.h"
#include "UserInterfaceMode.h"

namespace seb::settings::userinterface {

struct UserInterfaceSettings
{
    ActionCenterSettings ActionCenter;
    LockScreenSettings LockScreen;
    TaskbarSettings Taskbar;
    UserInterfaceMode Mode = UserInterfaceMode::Desktop;
};

}  // namespace seb::settings::userinterface
