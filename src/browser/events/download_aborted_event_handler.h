#pragma once

#include <functional>

namespace seb::browser::events {
using DownloadAbortedEventHandler = std::function<void()>;
}
