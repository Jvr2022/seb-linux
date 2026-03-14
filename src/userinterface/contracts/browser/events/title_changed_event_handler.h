#pragma once

#include <functional>
#include <QString>

namespace seb::userinterface::contracts::browser::events {
using TitleChangedEventHandler = std::function<void(const QString &)>;
}
