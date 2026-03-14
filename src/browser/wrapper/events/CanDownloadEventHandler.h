#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using CanDownloadEventHandler = std::function<void(const QString &)>;
}
