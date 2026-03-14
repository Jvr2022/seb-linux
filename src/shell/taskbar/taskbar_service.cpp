#include "taskbar_service.h"

namespace seb::shell::taskbar {

TaskbarModel TaskbarService::buildModel(const seb::SebSettings &settings, bool hasBattery)
{
    TaskbarModel model;
    model.visible = settings.taskbar.enableTaskbar;
    model.showApplicationInfo = settings.taskbar.showApplicationInfo;
    model.showApplicationLog = settings.taskbar.showApplicationLog;
    model.showProctoring = settings.taskbar.showProctoringNotification;
    model.showVerificator = settings.taskbar.showVerificator;
    model.showPower = hasBattery;
    model.showNetwork = settings.taskbar.showNetwork;
    model.showAudio = settings.taskbar.showAudio;
    model.showKeyboardLayout = settings.taskbar.showKeyboardLayout;
    model.showClock = settings.taskbar.showClock;
    model.showQuit = settings.security.allowTermination;
    return model;
}

}  // namespace seb::shell::taskbar
