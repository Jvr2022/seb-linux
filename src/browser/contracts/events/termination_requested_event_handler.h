#pragma once

#include <functional>

namespace seb::browser::contracts::events {

using TerminationRequestedEventHandler = std::function<void()>;

}  // namespace seb::browser::contracts::events
