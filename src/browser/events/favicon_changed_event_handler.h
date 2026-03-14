#pragma once

#include <functional>
#include <QString>

namespace seb::browser::events {
using FaviconChangedEventHandler = std::function<void(const QString &)>;
}
