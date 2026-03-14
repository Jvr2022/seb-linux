#include "external_application_instance.h"

#include "application_window.h"
#include "../shell/taskbar/platform/command_helper.h"

#include <QFileInfo>
#include <algorithm>
#include <QProcess>
#include <QTimer>

namespace seb::applications {

ExternalApplicationInstance::ExternalApplicationInstance(qint64 processId, QString executablePath, QObject *parent)
    : QObject(parent)
    , processId_(processId)
    , executablePath_(std::move(executablePath))
    , fallbackIconPath_(executablePath_)
{
}

ExternalApplicationInstance::~ExternalApplicationInstance()
{
    qDeleteAll(windows_);
}

QList<ApplicationWindow *> ExternalApplicationInstance::windows() const
{
    return windows_;
}

qint64 ExternalApplicationInstance::processId() const
{
    return processId_;
}

void ExternalApplicationInstance::attachProcess(QProcess *process)
{
    process_ = process;
    if (!process_) {
        return;
    }

    connect(process_, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
        emit terminated(processId_);
    });
}

void ExternalApplicationInstance::startMonitoring(int intervalMs)
{
    if (!timer_) {
        timer_ = new QTimer(this);
        connect(timer_, &QTimer::timeout, this, &ExternalApplicationInstance::refreshWindows);
    }
    timer_->start(intervalMs);
    refreshWindows();
}

void ExternalApplicationInstance::terminate(bool force)
{
    if (!process_) {
        return;
    }

    if (force) {
        process_->kill();
    } else {
        process_->terminate();
    }
}

void ExternalApplicationInstance::refreshWindows()
{
    const QString output = seb::shell::taskbar::platform::runCommand(
        QStringLiteral("xdotool"),
        {QStringLiteral("search"), QStringLiteral("--onlyvisible"), QStringLiteral("--pid"), QString::number(processId_)});
    const QStringList ids = output.split('\n', Qt::SkipEmptyParts);
    bool changed = false;

    for (int index = windows_.size() - 1; index >= 0; --index) {
        ApplicationWindow *window = windows_.at(index);
        if (!ids.contains(window->windowId())) {
            windows_.removeAt(index);
            delete window;
            changed = true;
        }
    }

    for (const QString &id : ids) {
        auto existing = std::find_if(windows_.begin(), windows_.end(), [&id](ApplicationWindow *window) {
            return window->windowId() == id;
        });
        const QString title = resolveWindowTitle(id);

        if (existing == windows_.end()) {
            windows_.push_back(new ApplicationWindow(id, title, fallbackIconPath_, this));
            changed = true;
        } else {
            (*existing)->update(title, fallbackIconPath_);
        }
    }

    if (changed) {
        emit windowsChanged();
    }
}

QString ExternalApplicationInstance::resolveWindowTitle(const QString &windowId) const
{
    const QString title = seb::shell::taskbar::platform::runCommand(
        QStringLiteral("xdotool"),
        {QStringLiteral("getwindowname"), windowId});
    if (!title.isEmpty()) {
        return title;
    }

    return QFileInfo(executablePath_).completeBaseName();
}

}  // namespace seb::applications
