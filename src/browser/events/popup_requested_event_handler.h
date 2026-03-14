#pragma once

#include "popup_requested_event_args.h"

#include <functional>

namespace seb::browser::events {
using PopupRequestedEventHandler = std::function<void(const PopupRequestedEventArgs &)>;
}
