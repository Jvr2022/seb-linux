#pragma once

#include "ResourceRequestEventArgs.h"

#include <functional>

namespace seb::browser::wrapper::events {
using ResourceRequestEventHandler = std::function<void(const ResourceRequestEventArgs &)>;
}
