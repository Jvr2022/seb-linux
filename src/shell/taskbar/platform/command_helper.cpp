#include "command_helper.h"

#include <QProcess>
#include <QProcessEnvironment>
#include <QTimer>
#include <memory>
#include <utility>

namespace seb::shell::taskbar::platform {

void runCommandAsync(QObject *context, const QString &program, const QStringList &arguments,
                     std::function<void(QString)> done, int timeoutMs)
{
    auto *process = new QProcess(context);
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process->setProcessEnvironment(environment);
    process->setStandardErrorFile(QProcess::nullDevice());
    auto completed = std::make_shared<bool>(false);
    const auto finish = [process, done = std::move(done), completed](bool success) {
        if (std::exchange(*completed, true)) return;
        const auto output = process->isOpen() ? process->readAllStandardOutput() : QByteArray();
        process->deleteLater();
        done(success && output.size() <= 1024 * 1024 ? QString::fromUtf8(output).trimmed() : QString());
    };
    QObject::connect(process, &QProcess::finished, context, [finish](int exitCode, QProcess::ExitStatus status) {
        finish(exitCode == 0 && status == QProcess::NormalExit);
    });
    QObject::connect(process, &QProcess::errorOccurred, context, [finish](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) finish(false);
    });
    QObject::connect(context, &QObject::destroyed, process, [process] { process->kill(); });
    auto *timeout = new QTimer(process);
    timeout->setSingleShot(true);
    QObject::connect(timeout, &QTimer::timeout, process, [process] { process->kill(); });
    process->start(program, arguments);
    timeout->start(qMax(1, timeoutMs));
}

QString runCommand(const QString &program, const QStringList &arguments, int timeoutMs)
{
    QProcess process;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("LC_ALL"), QStringLiteral("C"));
    process.setProcessEnvironment(environment);
    process.start(program, arguments);
    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(200);
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

}  // namespace seb::shell::taskbar::platform
