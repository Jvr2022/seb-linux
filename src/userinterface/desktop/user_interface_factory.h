#pragma once

#include "../contracts/i_user_interface_factory.h"
#include "../contracts/shell/i_taskbar.h"
#include "file_system_dialog_factory.h"
#include "message_box_factory.h"
#include "window_factory.h"
#include "windows/splash_screen.h"

#include <memory>

namespace seb::userinterface::desktop {

class UserInterfaceFactory : public contracts::IUserInterfaceFactory
{
public:
    explicit UserInterfaceFactory(contracts::shell::ITaskbar *taskbar = nullptr);

    contracts::message_box::IMessageBox *messageBox() override;
    contracts::shell::ITaskbar *taskbar() override;
    contracts::windows::IRuntimeWindow *runtimeWindow() override;
    contracts::windows::ISplashScreen *splashScreen() override;

private:
    FileSystemDialogFactory fileSystemDialogFactory_;
    MessageBoxFactory messageBoxFactory_;
    WindowFactory windowFactory_;
    contracts::shell::ITaskbar *taskbar_ = nullptr;
    std::unique_ptr<windows::RuntimeWindow> runtimeWindow_;
    std::unique_ptr<windows::SplashScreen> splashScreen_;
};

}  // namespace seb::userinterface::desktop
