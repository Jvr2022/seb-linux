#include "lock_screen.h"
#include <QDialog>
namespace seb::userinterface::desktop::windows { LockScreen::LockScreen(QWidget *parent) : parent_(parent) {} contracts::windows::data::LockScreenResult LockScreen::exec() { QDialog dialog(parent_); contracts::windows::data::LockScreenResult r; r.accepted = (dialog.exec() == QDialog::Accepted); return r; } }
