#pragma once

#include <QStringList>
#include <functional>
#include <QObject>

namespace seb::shell::taskbar::platform {

void runCommandAsync(QObject *context, const QString &program, const QStringList &arguments,
                     std::function<void(QString)> done, int timeoutMs = 2000);

QString runCommand(const QString &program, const QStringList &arguments, int timeoutMs = 2000);

}  // namespace seb::shell::taskbar::platform
