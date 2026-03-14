#pragma once

#include "WindowPosition.h"

namespace seb::settings::browser {

struct WindowSettings
{
    int AbsoluteHeight = 0;
    int AbsoluteWidth = 0;
    int RelativeHeight = 100;
    int RelativeWidth = 100;
    bool AllowAddressBar = false;
    bool AllowBackwardNavigation = false;
    bool AllowDeveloperConsole = false;
    bool AllowForwardNavigation = false;
    bool AllowReloading = true;
    bool AllowMinimize = true;
    bool AlwaysOnTop = false;
    bool FullScreenMode = false;
    bool HideBrowserWindowToolbar = false;
    bool ShowMenuBar = false;
    bool ShowHomeButton = false;
    bool ShowReloadButton = false;
    bool ShowReloadWarning = false;
    bool ShowTitleBar = true;
    bool UseNativeBrowserToolbar = false;
    WindowPosition Position = WindowPosition::Center;
};

}  // namespace seb::settings::browser
