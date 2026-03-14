#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using ContextReleasedEventHandler = std::function<void()>;
}
