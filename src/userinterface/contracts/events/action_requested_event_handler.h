#pragma once

#include <functional>

namespace seb::userinterface::contracts::events {
using ActionRequestedEventHandler = std::function<void()>;
}
