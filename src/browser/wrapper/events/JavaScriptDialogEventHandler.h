#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using JavaScriptDialogEventHandler = std::function<void(const QString &)>;
}
