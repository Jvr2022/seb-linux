#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using PreKeyEventHandler = std::function<void(int)>;
}
