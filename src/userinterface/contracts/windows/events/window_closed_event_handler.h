#pragma once

#include <functional>

namespace seb::userinterface::contracts::windows::events {
using WindowClosedEventHandler = std::function<void()>;
}
