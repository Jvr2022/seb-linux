#pragma once

#include "data/server_failure_dialog_result.h"

namespace seb::userinterface::contracts::windows { class IServerFailureDialog { public: virtual ~IServerFailureDialog() = default; virtual data::ServerFailureDialogResult exec() = 0; }; }
