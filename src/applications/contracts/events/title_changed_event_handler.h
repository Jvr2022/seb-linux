#pragma once

#include <functional>
#include <QString>

namespace seb::applications::contracts::events {

using TitleChangedEventHandler = std::function<void(const QString &)>;

}  // namespace seb::applications::contracts::events
