#pragma once

#include <functional>

namespace seb::userinterface::contracts::proctoring::events {
using FullScreenChangedEventHandler = std::function<void(bool)>;
}
