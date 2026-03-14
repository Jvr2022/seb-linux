#pragma once

#include <functional>
#include <QString>

namespace seb::userinterface::contracts::browser::events {
using AddressChangedEventHandler = std::function<void(const QString &)>;
}
