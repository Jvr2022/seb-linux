#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using UncaughtExceptionEventHandler = std::function<void(const QString &)>;
}
