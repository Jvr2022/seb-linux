#pragma once

namespace seb::browser::responsibilities {

enum class BrowserTask
{
    InitializeBrowserConfiguration,
    InitializeCookies,
    InitializeFileSystem,
    InitializeIntegrity,
    InitializePreferences,
    CreateMainWindow,
    CloseAllWindows,
    FinalizeCookies,
    FinalizeFileSystem,
    FinalizeCache
};

}  // namespace seb::browser::responsibilities
