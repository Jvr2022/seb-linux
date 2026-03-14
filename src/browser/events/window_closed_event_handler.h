#pragma once

#include <functional>

namespace seb::browser::events {
using WindowClosedEventHandler = std::function<void()>;
}
