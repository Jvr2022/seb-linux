#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using FocusedNodeChangedEventHandler = std::function<void()>;
}
