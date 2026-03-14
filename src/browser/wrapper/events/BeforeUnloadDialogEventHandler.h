#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using BeforeUnloadDialogEventHandler = std::function<void(const QString &)>;
}
