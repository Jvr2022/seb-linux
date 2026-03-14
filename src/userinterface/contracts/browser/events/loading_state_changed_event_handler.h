#pragma once

#include <functional>

namespace seb::userinterface::contracts::browser::events {
using LoadingStateChangedEventHandler = std::function<void(bool)>;
}
