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
    ClipboardPolicy ClipboardPolicy = ClipboardPolicy::Isolated;
    bool DisableSessionChangeLockScreen = false;
    KioskMode KioskMode = KioskMode::None;
    QString QuitPasswordHash;
    QString ReconfigurationUrl;
    bool VerifyCursorConfiguration = false;
    bool VerifySessionIntegrity = false;
    QList<VersionRestriction> VersionRestrictions;
    VirtualMachinePolicy VirtualMachinePolicy = VirtualMachinePolicy::Allow;
};

}  // namespace seb::settings::security
