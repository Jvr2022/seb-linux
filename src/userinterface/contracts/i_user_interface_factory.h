#pragma once

#include "browser/i_browser_window.h"
#include "message_box/i_message_box.h"
#include "shell/i_taskbar.h"
#include "windows/i_runtime_window.h"
#include "windows/i_splash_screen.h"

namespace seb::userinterface::contracts {

class IUserInterfaceFactory
{
public:
    virtual ~IUserInterfaceFactory() = default;
    virtual message_box::IMessageBox *messageBox() = 0;
    virtual shell::ITaskbar *taskbar() = 0;
    virtual windows::IRuntimeWindow *runtimeWindow() = 0;
    virtual windows::ISplashScreen *splashScreen() = 0;
};

}  // namespace seb::userinterface::contracts
