#include "version_restriction_validator.h"

#include <QRegularExpression>

#include <array>

namespace seb::runtime {

namespace {

bool parseVersion(
    const QString &version,
    std::array<int, 4> &parsed)
{
    static const QRegularExpression pattern(
        QStringLiteral(R"(^([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+)$)"));
    const QRegularExpressionMatch match = pattern.match(version.trimmed());
    if (!match.hasMatch()) {
        return false;
    }

    for (int index = 0; index < 4; ++index) {
        bool ok = false;
        parsed[static_cast<std::size_t>(index)] =
            match.captured(index + 1).toInt(&ok);
        if (!ok || parsed[static_cast<std::size_t>(index)] < 0) {
            return false;
        }
    }

    return true;
}

}  // namespace

bool VersionRestrictionValidator::validate(
    const QList<settings::security::VersionRestriction> &restrictions,
    const QString &version,
    bool allianceEdition) const
{
    if (restrictions.isEmpty()) {
        return true;
    }

    std::array<int, 4> parsed{};
    if (!parseVersion(version, parsed)) {
        return false;
    }

    for (const auto &restriction : restrictions) {
        if (isFulfilled(
                restriction,
                parsed[0],
                parsed[1],
                parsed[2],
                parsed[3],
                allianceEdition)) {
            return true;
        }
    }

    return false;
}

bool VersionRestrictionValidator::isFulfilled(
    const settings::security::VersionRestriction &restriction,
    int major,
    int minor,
    int patch,
    int build,
    bool allianceEdition) const
{
    if (restriction.RequiresAllianceEdition && !allianceEdition) {
        return false;
    }

    if (!restriction.IsMinimumRestriction) {
        if (major != restriction.Major || minor != restriction.Minor) {
            return false;
        }
        if (restriction.Patch.has_value() && patch != *restriction.Patch) {
            return false;
        }
        if (restriction.Build.has_value() && build != *restriction.Build) {
            return false;
        }
        return true;
    }

    bool fulfilled = true;
    fulfilled &= restriction.Major <= major;

    if (restriction.Major == major) {
        fulfilled &= restriction.Minor <= minor;

        if (restriction.Minor == minor) {
            fulfilled &= !restriction.Patch.has_value()
                || *restriction.Patch <= patch;

            if (restriction.Patch.has_value()
                && *restriction.Patch == patch) {
                fulfilled &= !restriction.Build.has_value()
                    || *restriction.Build <= build;
            }
        }
    }

    return fulfilled;
}

}  // namespace seb::runtime
