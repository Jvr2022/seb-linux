#include "security_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {

void SecurityDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &security = settings.Security;
    if (key == keys::Security::AdminPasswordHash) security.AdminPasswordHash = string(value);
    else if (key == keys::Security::AllowApplicationLog) security.AllowApplicationLogAccess = boolean(value, security.AllowApplicationLogAccess);
    else if (key == keys::Security::AllowReconfiguration) security.AllowReconfiguration = boolean(value, security.AllowReconfiguration);
    else if (key == keys::Security::AllowStickyKeys) security.AllowStickyKeys = boolean(value, security.AllowStickyKeys);
    else if (key == keys::Security::AllowTermination) security.AllowTermination = boolean(value, security.AllowTermination);
    else if (key == keys::Security::AllowVirtualMachine) security.VirtualMachinePolicy = boolean(value) ? seb::settings::security::VirtualMachinePolicy::Allow : seb::settings::security::VirtualMachinePolicy::Deny;
    else if (key == keys::Security::AllowWindowCapture) security.AllowWindowCapture = boolean(value, security.AllowWindowCapture);
    else if (key == keys::Security::ClipboardPolicy) {
        const int policy = integer(value);
        security.ClipboardPolicy = policy == 0 ? seb::settings::security::ClipboardPolicy::Allow : policy == 1 ? seb::settings::security::ClipboardPolicy::Block : seb::settings::security::ClipboardPolicy::Isolated;
    } else if (key == keys::Security::DisableSessionChangeLockScreen) security.DisableSessionChangeLockScreen = boolean(value, security.DisableSessionChangeLockScreen);
    else if (key == keys::Security::QuitPasswordHash) security.QuitPasswordHash = string(value);
    else if (key == keys::Security::ReconfigurationUrl) security.ReconfigurationUrl = string(value);
    else if (key == keys::Security::VerifyCursorConfiguration) security.VerifyCursorConfiguration = boolean(value, security.VerifyCursorConfiguration);
    else if (key == keys::Security::VerifySessionIntegrity) security.VerifySessionIntegrity = boolean(value, security.VerifySessionIntegrity);
    else if (key == keys::Security::VersionRestrictions) {
        for (const QVariant &entry : value.toList()) {
            seb::settings::security::VersionRestriction item;
            const QString restriction = string(entry);
            const QStringList parts = restriction.split(QLatin1Char('.'));
            if (parts.size() > 0 && parts.first().compare(QStringLiteral("win"), Qt::CaseInsensitive) != 0) continue;
            if (parts.size() > 1) item.Major = parts.at(1).toInt();
            if (parts.size() > 2) item.Minor = parts.at(2).toInt();
            if (parts.size() > 3) item.Patch = parts.at(3).toInt();
            if (parts.size() > 4) item.Build = parts.at(4).toInt();
            item.IsMinimumRestriction = restriction.contains(QStringLiteral("min"), Qt::CaseInsensitive);
            item.RequiresAllianceEdition = restriction.contains(QStringLiteral("AE"), Qt::CaseInsensitive);
            item.MinimumVersion = restriction;
            security.VersionRestrictions.push_back(item);
        }
    }
}

void SecurityDataMapper::mapGlobal(const QVariantMap &data, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    const bool hasNewDesktop = data.contains(keys::Security::KioskModeCreateNewDesktop);
    const bool hasKillShell = data.contains(keys::Security::KioskModeDisableExplorerShell);
    if (hasNewDesktop || hasKillShell) {
        const bool newDesktop = boolean(data.value(keys::Security::KioskModeCreateNewDesktop));
        const bool killShell = boolean(data.value(keys::Security::KioskModeDisableExplorerShell));
        if (killShell) settings.Security.KioskMode = seb::settings::security::KioskMode::DisableExplorerShell;
        else if (newDesktop) settings.Security.KioskMode = seb::settings::security::KioskMode::CreateNewDesktop;
        else if (hasNewDesktop && hasKillShell) settings.Security.KioskMode = seb::settings::security::KioskMode::None;
    }
    if (data.contains(keys::Security::AllowApplicationLog)) {
        settings.Security.AllowApplicationLogAccess = helpers::boolean(data.value(keys::Security::AllowApplicationLog));
    }
    settings.UserInterface.ActionCenter.ShowApplicationLog = settings.Security.AllowApplicationLogAccess;
    if (!settings.Security.AllowApplicationLogAccess) settings.UserInterface.Taskbar.ShowApplicationLog = false;
}

}  // namespace seb::configuration::configuration_data::data_mapping
