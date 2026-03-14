#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using DownloadUpdatedEventHandler = std::function<void(const QString &, int)>;
}
