#pragma once

#include <functional>
#include <QStringList>

namespace seb::browser::wrapper::events {
using FaviconUrlChangedEventHandler = std::function<void(const QStringList &)>;
}
