#pragma once

#include <functional>

namespace seb::applications::contracts::events {

using WindowsChangedEventHandler = std::function<void()>;

}  // namespace seb::applications::contracts::events
