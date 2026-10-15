#pragma once

#include "taskbar_model.h"
#include "../../settings/Security/SecuritySettings.h"
#include "../../settings/Proctoring/ProctoringSettings.h"
#include "../../settings/UserInterface/TaskbarSettings.h"

namespace seb::shell::taskbar {

class TaskbarService
{
public:
    static TaskbarModel buildModel(
        const seb::settings::userinterface::TaskbarSettings &taskbar,
        const seb::settings::security::SecuritySettings &security,
        const seb::settings::proctoring::ProctoringSettings &proctoring,
        bool hasBattery);
};

}  // namespace seb::shell::taskbar
