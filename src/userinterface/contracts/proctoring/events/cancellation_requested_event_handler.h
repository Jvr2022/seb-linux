#pragma once

#include <functional>

namespace seb::userinterface::contracts::proctoring::events {
using CancellationRequestedEventHandler = std::function<void()>;
}
