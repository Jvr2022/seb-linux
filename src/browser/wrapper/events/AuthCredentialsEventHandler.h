#pragma once

#include <functional>
#include <QString>

namespace seb::browser::wrapper::events {
using AuthCredentialsEventHandler = std::function<void(const QString &, int)>;
}
