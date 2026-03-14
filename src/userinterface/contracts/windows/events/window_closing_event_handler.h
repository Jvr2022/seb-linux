#pragma once

#include <functional>

namespace seb::userinterface::contracts::windows::events {
using WindowClosingEventHandler = std::function<bool()>;
}
