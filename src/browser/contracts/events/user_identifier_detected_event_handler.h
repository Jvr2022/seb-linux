#pragma once

#include <QString>
#include <functional>

namespace seb::browser::contracts::events {

using UserIdentifierDetectedEventHandler = std::function<void(const QString &identifier)>;

}  // namespace seb::browser::contracts::events
