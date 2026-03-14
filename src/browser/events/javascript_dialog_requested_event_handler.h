#pragma once

#include "javascript_dialog_requested_event_args.h"

#include <functional>

namespace seb::browser::events {
using JavaScriptDialogRequestedEventHandler = std::function<void(const JavaScriptDialogRequestedEventArgs &)>;
}
