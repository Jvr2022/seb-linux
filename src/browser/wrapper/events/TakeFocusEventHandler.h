#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using TakeFocusEventHandler = std::function<void(bool)>;
}
