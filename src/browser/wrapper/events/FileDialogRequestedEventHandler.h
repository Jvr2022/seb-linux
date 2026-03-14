#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using FileDialogRequestedEventHandler = std::function<void(const QString &)>;
}
