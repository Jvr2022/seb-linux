#pragma once

#include <QStringList>

namespace seb::shell::taskbar::platform {

QString runCommand(const QString &program, const QStringList &arguments, int timeoutMs = 2000);

}  // namespace seb::shell::taskbar::platform
