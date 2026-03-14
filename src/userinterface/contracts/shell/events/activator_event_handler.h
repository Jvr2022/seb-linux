#pragma once

#include <functional>

namespace seb::userinterface::contracts::shell::events {
using ActivatorEventHandler = std::function<void()>;
}
