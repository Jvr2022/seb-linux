#pragma once
#include "lock_screen_option.h"
namespace seb::userinterface::contracts::windows::data { struct LockScreenResult { bool accepted = false; LockScreenOption option = LockScreenOption::None; }; }
