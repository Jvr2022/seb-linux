#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using BeforeBrowseEventHandler = std::function<void(const QString &)>;
}
