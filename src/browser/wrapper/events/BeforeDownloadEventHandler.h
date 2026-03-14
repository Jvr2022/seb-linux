#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using BeforeDownloadEventHandler = std::function<void(const QString &)>;
}
