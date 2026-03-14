#pragma once

#include "dialog_requested_event_args.h"

#include <functional>

namespace seb::browser::events {
using DialogRequestedEventHandler = std::function<void(const DialogRequestedEventArgs &)>;
}
