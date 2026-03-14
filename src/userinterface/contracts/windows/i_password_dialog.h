#pragma once

#include "data/password_dialog_result.h"

namespace seb::userinterface::contracts::windows { class IPasswordDialog { public: virtual ~IPasswordDialog() = default; virtual data::PasswordDialogResult exec() = 0; }; }
