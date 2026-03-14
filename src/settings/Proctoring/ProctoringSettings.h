#pragma once

#include "MetaDataSettings.h"
#include "ScreenProctoringSettings.h"

namespace seb::settings::proctoring {

struct ProctoringSettings
{
    bool Enabled = false;
    MetaDataSettings MetaData;
    ScreenProctoringSettings ScreenProctoring;
};

}  // namespace seb::settings::proctoring
