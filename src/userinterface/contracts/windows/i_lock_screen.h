#pragma once

#include "data/lock_screen_result.h"

namespace seb::userinterface::contracts::windows { class ILockScreen { public: virtual ~ILockScreen() = default; virtual data::LockScreenResult exec() = 0; }; }
