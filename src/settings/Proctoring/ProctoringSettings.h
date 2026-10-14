#pragma once

#include "MetaDataSettings.h"
#include "ScreenProctoringSettings.h"

namespace seb::settings::proctoring {

struct ProctoringSettings
{
    QString ClientId;
    QString ClientSecret;
    bool Enabled = false;
    QString ServiceUrl;
    bool ShowDisclaimer = true;
    bool ShowTaskbarNotification = true;
    MetaDataSettings MetaData;
    ScreenProctoringSettings ScreenProctoring;
};

}  // namespace seb::settings::proctoring
