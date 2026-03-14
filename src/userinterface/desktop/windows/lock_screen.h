#pragma once
#include "../../contracts/windows/i_lock_screen.h"
class QWidget;
namespace seb::userinterface::desktop::windows { class LockScreen : public contracts::windows::ILockScreen { public: explicit LockScreen(QWidget *parent = nullptr); contracts::windows::data::LockScreenResult exec() override; private: QWidget *parent_ = nullptr; }; }
