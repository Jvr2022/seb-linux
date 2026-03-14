#pragma once

#include <functional>
#include <QString>

namespace seb::browser::events {
using ClipboardChangedEventHandler = std::function<void(const QString &)>;
}
