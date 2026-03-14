#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using KeyEventHandler = std::function<void(int)>;
}
