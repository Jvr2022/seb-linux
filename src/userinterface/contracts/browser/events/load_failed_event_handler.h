#pragma once

#include <functional>
#include <QString>

namespace seb::userinterface::contracts::browser::events {
using LoadFailedEventHandler = std::function<void(const QString &)>;
}
