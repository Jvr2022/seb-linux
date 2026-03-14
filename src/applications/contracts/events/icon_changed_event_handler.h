#pragma once

#include <functional>
#include <QString>

namespace seb::applications::contracts::events {

using IconChangedEventHandler = std::function<void(const QString &)>;

}  // namespace seb::applications::contracts::events
