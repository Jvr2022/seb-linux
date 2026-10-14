#pragma once
namespace seb::settings::monitoring {
struct DisplaySettings {
    int AllowedDisplays = 1;
    bool AlwaysOn = true;
    bool DisableMultipleDisplays = false;
    bool EnforcePrimaryDisplay = false;
    bool IgnoreError = false;
    bool InternalDisplayOnly = false;
};
}
