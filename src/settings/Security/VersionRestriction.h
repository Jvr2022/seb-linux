#pragma once

#include <QString>
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
    QString MinimumVersion;
    QString MaximumVersion;
};

}  // namespace seb::settings::security
