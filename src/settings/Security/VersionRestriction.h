#pragma once

#include <optional>

namespace seb::settings::security {

struct VersionRestriction
{
    int Major = 0;
    int Minor = 0;
    std::optional<int> Patch;
    std::optional<int> Build;
    bool IsMinimumRestriction = false;
    bool RequiresAllianceEdition = false;
};

}  // namespace seb::settings::security
