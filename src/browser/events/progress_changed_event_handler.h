#pragma once

#include <functional>

namespace seb::browser::events {
using ProgressChangedEventHandler = std::function<void(int)>;
}
