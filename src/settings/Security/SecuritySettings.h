#pragma once

#include "ClipboardPolicy.h"
#include "KioskMode.h"
#include "VersionRestriction.h"
#include "VirtualMachinePolicy.h"

#include <QList>
#include <QString>

namespace seb::settings::security {

struct SecuritySettings
{
    QString AdminPasswordHash;
    bool AllowApplicationLogAccess = false;
    bool AllowReconfiguration = false;
    bool AllowStickyKeys = false;
    bool AllowTermination = true;
    bool AllowWindowCapture = false;
    seb::settings::security::ClipboardPolicy ClipboardPolicy = seb::settings::security::ClipboardPolicy::Isolated;
    bool DisableSessionChangeLockScreen = false;
    seb::settings::security::KioskMode KioskMode = seb::settings::security::KioskMode::None;
    QString QuitPasswordHash;
    QString ReconfigurationUrl;
    QString Version;
    bool VerifyCursorConfiguration = false;
    bool VerifySessionIntegrity = false;
    QList<VersionRestriction> VersionRestrictions;
    seb::settings::security::VirtualMachinePolicy VirtualMachinePolicy = seb::settings::security::VirtualMachinePolicy::Allow;
};

}  // namespace seb::settings::security
