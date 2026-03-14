#pragma once

#include <QString>

namespace seb::settings::security {

struct VersionRestriction
{
    QString MinimumVersion;
    QString MaximumVersion;
};

}  // namespace seb::settings::security
