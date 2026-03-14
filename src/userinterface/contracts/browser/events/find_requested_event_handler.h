#pragma once

#include <functional>
#include <QString>

namespace seb::userinterface::contracts::browser::events {
using FindRequestedEventHandler = std::function<void(const QString &, bool, bool)>;
}
