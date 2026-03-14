#pragma once

#include <functional>
#include <QString>

namespace seb::browser::events {
using UrlEventHandler = std::function<void(const QString &)>;
}
