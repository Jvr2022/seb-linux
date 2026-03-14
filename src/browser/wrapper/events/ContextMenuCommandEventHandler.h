#pragma once

#include <functional>

namespace seb::browser::wrapper::events {
using ContextMenuCommandEventHandler = std::function<void(int)>;
}
