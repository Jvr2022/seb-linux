#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using OpenUrlFromTabEventHandler = std::function<void(const QString &)>;
}
