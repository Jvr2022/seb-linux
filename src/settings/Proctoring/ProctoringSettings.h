#pragma once

#include "ScreenProctoringSettings.h"

namespace seb::settings::proctoring {

struct ProctoringSettings
{
    bool Enabled = false;
    bool ShowDisclaimer = true;
    bool ShowTaskbarNotification = true;
    ScreenProctoringSettings ScreenProctoring;
};

}  // namespace seb::settings::proctoring
