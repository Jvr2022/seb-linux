#pragma once

#include <functional>

namespace seb::browser::contracts::events {

using TabPressedEventHandler = std::function<void(bool forward)>;

}  // namespace seb::browser::contracts::events
