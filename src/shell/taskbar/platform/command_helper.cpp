#include "command_helper.h"

#include <QProcess>

namespace seb::shell::taskbar::platform {

QString runCommand(const QString &program, const QStringList &arguments, int timeoutMs)
{
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(200);
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

}  // namespace seb::shell::taskbar::platform
