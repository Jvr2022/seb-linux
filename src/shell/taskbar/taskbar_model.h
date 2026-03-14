#pragma once

#include "../../seb_settings.h"

namespace seb::shell::taskbar {

struct TaskbarModel
{
    bool visible = true;
    bool showApplicationInfo = false;
    bool showApplicationLog = false;
    bool showProctoring = false;
    bool showVerificator = false;
    bool showPower = false;
    bool showNetwork = false;
    bool showAudio = false;
    bool showKeyboardLayout = false;
    bool showClock = false;
    bool showQuit = true;
};

}  // namespace seb::shell::taskbar
