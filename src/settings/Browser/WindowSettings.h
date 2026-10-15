#pragma once

#include <optional>

#include "WindowPosition.h"
#include "UrlPolicy.h"

namespace seb::settings::browser {

struct WindowSettings
{
    std::optional<int> AbsoluteHeight;
    std::optional<int> AbsoluteWidth;
    std::optional<int> RelativeHeight;
    std::optional<int> RelativeWidth;
    bool AllowAddressBar = false;
    bool AllowBackwardNavigation = false;
    bool AllowDeveloperConsole = false;
    bool AllowForwardNavigation = false;
    bool AllowReloading = true;
    bool FullScreenMode = false;
    bool ShowHomeButton = false;
    bool ShowReloadButton = false;
    bool ShowReloadWarning = false;
    bool ShowToolbar = true;
    seb::settings::browser::UrlPolicy UrlPolicy = seb::settings::browser::UrlPolicy::SameWindow;
    WindowPosition Position = WindowPosition::Center;
};

}  // namespace seb::settings::browser
