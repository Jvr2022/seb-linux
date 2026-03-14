#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using LoadingProgressChangedEventHandler = std::function<void(double)>;
}
