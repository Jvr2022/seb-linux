#include "window_factory.h"

namespace seb::userinterface::desktop {

std::unique_ptr<windows::AboutWindow> WindowFactory::createAboutWindow() { return std::make_unique<windows::AboutWindow>(); }
std::unique_ptr<windows::ActionCenter> WindowFactory::createActionCenter() { return std::make_unique<windows::ActionCenter>(); }
std::unique_ptr<windows::BrowserWindow> WindowFactory::createBrowserWindow(contracts::browser::IBrowserControl *) { return std::make_unique<windows::BrowserWindow>(); }
std::unique_ptr<windows::CredentialsDialog> WindowFactory::createCredentialsDialog() { return std::make_unique<windows::CredentialsDialog>(); }
std::unique_ptr<windows::ExamSelectionDialog> WindowFactory::createExamSelectionDialog() { return std::make_unique<windows::ExamSelectionDialog>(); }
std::unique_ptr<windows::LockScreen> WindowFactory::createLockScreen() { return std::make_unique<windows::LockScreen>(); }
std::unique_ptr<windows::LogWindow> WindowFactory::createLogWindow() { return std::make_unique<windows::LogWindow>(); }
std::unique_ptr<windows::PasswordDialog> WindowFactory::createPasswordDialog() { return std::make_unique<windows::PasswordDialog>(); }
std::unique_ptr<windows::ProctoringFinalizationDialog> WindowFactory::createProctoringFinalizationDialog() { return std::make_unique<windows::ProctoringFinalizationDialog>(); }
std::unique_ptr<windows::ProctoringWindow> WindowFactory::createProctoringWindow(contracts::proctoring::IProctoringControl *) { return std::make_unique<windows::ProctoringWindow>(); }
std::unique_ptr<windows::RuntimeWindow> WindowFactory::createRuntimeWindow() { return std::make_unique<windows::RuntimeWindow>(); }
std::unique_ptr<windows::ServerFailureDialog> WindowFactory::createServerFailureDialog() { return std::make_unique<windows::ServerFailureDialog>(); }
std::unique_ptr<windows::SplashScreen> WindowFactory::createSplashScreen() { return std::make_unique<windows::SplashScreen>(); }
std::unique_ptr<windows::TaskbarWindow> WindowFactory::createTaskbarWindow() { return std::make_unique<windows::TaskbarWindow>(); }
std::unique_ptr<windows::Taskview> WindowFactory::createTaskview() { return std::make_unique<windows::Taskview>(); }
std::unique_ptr<windows::VerificatorOverlay> WindowFactory::createVerificatorOverlay() { return std::make_unique<windows::VerificatorOverlay>(); }

}  // namespace seb::userinterface::desktop
