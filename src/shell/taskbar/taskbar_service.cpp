#include "taskbar_service.h"

namespace seb::shell::taskbar {

TaskbarModel TaskbarService::buildModel(
    const seb::settings::userinterface::TaskbarSettings &taskbar,
    const seb::settings::security::SecuritySettings &security,
    const seb::settings::proctoring::ProctoringSettings &proctoring,
    bool hasBattery)
{
    TaskbarModel model;
    model.visible = taskbar.EnableTaskbar;
    model.showApplicationInfo = taskbar.ShowApplicationInfo;
    model.showApplicationLog = taskbar.ShowApplicationLog;
    model.showProctoring = proctoring.ShowTaskbarNotification;
    model.showVerificator = taskbar.ShowVerificator;
    model.showPower = hasBattery;
    model.showNetwork = taskbar.ShowNetwork;
    model.showAudio = taskbar.ShowAudio;
    model.showKeyboardLayout = taskbar.ShowKeyboardLayout;
    model.showClock = taskbar.ShowClock;
    model.showQuit = security.AllowTermination;
    return model;
}

}  // namespace seb::shell::taskbar
