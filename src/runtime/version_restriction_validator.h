#pragma once

#include "settings/Security/VersionRestriction.h"

#include <QList>
#include <QString>

namespace seb::runtime {

class VersionRestrictionValidator
{
public:
    bool validate(
        const QList<settings::security::VersionRestriction> &restrictions,
        const QString &version,
        bool allianceEdition = false) const;

private:
    bool isFulfilled(
        const settings::security::VersionRestriction &restriction,
        int major,
        int minor,
        int patch,
        int build,
        bool allianceEdition) const;
};

}  // namespace seb::runtime
