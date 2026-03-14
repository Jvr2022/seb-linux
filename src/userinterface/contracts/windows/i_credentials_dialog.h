#pragma once

#include "data/credentials_dialog_result.h"

namespace seb::userinterface::contracts::windows { class ICredentialsDialog { public: virtual ~ICredentialsDialog() = default; virtual data::CredentialsDialogResult exec() = 0; }; }
