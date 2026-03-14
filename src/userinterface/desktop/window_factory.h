#pragma once

#include "../contracts/browser/i_browser_control.h"
#include "../contracts/browser/i_browser_window.h"
#include "../contracts/proctoring/i_proctoring_control.h"
#include "../contracts/shell/i_action_center.h"
#include "../contracts/shell/i_taskbar.h"
#include "../contracts/shell/i_taskview.h"
#include "../contracts/windows/i_credentials_dialog.h"
#include "../contracts/windows/i_exam_selection_dialog.h"
#include "../contracts/windows/i_lock_screen.h"
#include "../contracts/windows/i_password_dialog.h"
#include "../contracts/windows/i_runtime_window.h"
#include "../contracts/windows/i_server_failure_dialog.h"
#include "../contracts/windows/i_splash_screen.h"
#include "../contracts/windows/i_verificator_overlay.h"
#include "windows/about_window.h"
#include "windows/action_center.h"
#include "windows/browser_window.h"
#include "windows/credentials_dialog.h"
#include "windows/exam_selection_dialog.h"
#include "windows/lock_screen.h"
#include "windows/log_window.h"
#include "windows/password_dialog.h"
#include "windows/proctoring_finalization_dialog.h"
#include "windows/proctoring_window.h"
#include "windows/runtime_window.h"
#include "windows/server_failure_dialog.h"
#include "windows/splash_screen.h"
#include "windows/taskbar_window.h"
#include "windows/taskview.h"
#include "windows/verificator_overlay.h"

#include <memory>

namespace seb::userinterface::desktop {

class WindowFactory
{
public:
    WindowFactory() = default;

    std::unique_ptr<windows::AboutWindow> createAboutWindow();
    std::unique_ptr<windows::ActionCenter> createActionCenter();
    std::unique_ptr<windows::BrowserWindow> createBrowserWindow(contracts::browser::IBrowserControl *control = nullptr);
    std::unique_ptr<windows::CredentialsDialog> createCredentialsDialog();
    std::unique_ptr<windows::ExamSelectionDialog> createExamSelectionDialog();
    std::unique_ptr<windows::LockScreen> createLockScreen();
    std::unique_ptr<windows::LogWindow> createLogWindow();
    std::unique_ptr<windows::PasswordDialog> createPasswordDialog();
    std::unique_ptr<windows::ProctoringFinalizationDialog> createProctoringFinalizationDialog();
    std::unique_ptr<windows::ProctoringWindow> createProctoringWindow(contracts::proctoring::IProctoringControl *control = nullptr);
    std::unique_ptr<windows::RuntimeWindow> createRuntimeWindow();
    std::unique_ptr<windows::ServerFailureDialog> createServerFailureDialog();
    std::unique_ptr<windows::SplashScreen> createSplashScreen();
    std::unique_ptr<windows::TaskbarWindow> createTaskbarWindow();
    std::unique_ptr<windows::Taskview> createTaskview();
    std::unique_ptr<windows::VerificatorOverlay> createVerificatorOverlay();
};

}  // namespace seb::userinterface::desktop
