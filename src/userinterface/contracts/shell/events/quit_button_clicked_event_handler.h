#pragma once

#include <functional>

namespace seb::userinterface::contracts::shell::events {
using QuitButtonClickedEventHandler = std::function<void()>;
}
