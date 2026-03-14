#include "user_interface_factory.h"

namespace seb::userinterface::desktop {

UserInterfaceFactory::UserInterfaceFactory(contracts::shell::ITaskbar *taskbar)
    : taskbar_(taskbar),
      runtimeWindow_(windowFactory_.createRuntimeWindow()),
      splashScreen_(windowFactory_.createSplashScreen())
{
}

contracts::message_box::IMessageBox *UserInterfaceFactory::messageBox() { return &messageBoxFactory_; }
contracts::shell::ITaskbar *UserInterfaceFactory::taskbar() { return taskbar_; }
contracts::windows::IRuntimeWindow *UserInterfaceFactory::runtimeWindow() { return runtimeWindow_.get(); }
contracts::windows::ISplashScreen *UserInterfaceFactory::splashScreen() { return splashScreen_.get(); }

}  // namespace seb::userinterface::desktop
