#include "security_service.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QTimerEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

#include <sys/ptrace.h>
#include <unistd.h>

namespace seb::security {

SecurityService::SecurityService(QObject *parent)
    : QObject(parent)
{
}

bool SecurityService::isVirtualMachine() const
{
    // Check for common VM artifacts in /proc/cpuinfo and dmesg-like files
    QFile cpuinfo("/proc/cpuinfo");
    if (cpuinfo.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = cpuinfo.readAll();
        if (content.contains("hypervisor", Qt::CaseInsensitive) ||
            content.contains("VMware", Qt::CaseInsensitive) ||
            content.contains("QEMU", Qt::CaseInsensitive) ||
            content.contains("VirtualBox", Qt::CaseInsensitive)) {
            return true;
        }
    }

    // Check system vendor
    QFile vendor("/sys/class/dmi/id/sys_vendor");
    if (vendor.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString content = vendor.readAll().trimmed();
        if (content.contains("VMware", Qt::CaseInsensitive) ||
            content.contains("QEMU", Qt::CaseInsensitive) ||
            content.contains("innotek", Qt::CaseInsensitive) || // VirtualBox
            content.contains("Microsoft Corporation", Qt::CaseInsensitive)) { // Hyper-V
            return true;
        }
    }

    return false;
}

bool SecurityService::isDebuggerAttached() const
{
    // Try to ptrace ourselves - if it fails, someone else is likely tracing us
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) < 0) {
        return true;
    }
    ptrace(PTRACE_DETACH, 0, 1, 0);

    // Also check TracerPid in /proc/self/status
    QFile status("/proc/self/status");
    if (status.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&status);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("TracerPid:")) {
                int tracerPid = line.section(':', 1).trimmed().toInt();
                if (tracerPid != 0) {
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

QStringList SecurityService::detectProhibitedProcesses() const
{
    QStringList prohibited = {
        "discord", "slack", "teamviewer", "anydesk", "obs", "simplescreenrecorder",
        "wireshark", "tcpdump", "gdb", "strace"
    };

    QStringList detected;
    QDir proc("/proc");
    QStringList pids = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &pid : pids) {
        bool ok;
        pid.toInt(&ok);
        if (!ok) continue;

        QFile cmdline(QString("/proc/%1/cmdline").arg(pid));
        if (cmdline.open(QIODevice::ReadOnly)) {
            QString name = QString::fromLocal8Bit(cmdline.readAll()).section('\0', 0, 0);
            for (const QString &p : prohibited) {
                if (name.contains(p, Qt::CaseInsensitive)) {
                    detected << name;
                }
            }
        }
    }

    return detected;
}

void SecurityService::startMonitoring()
{
    if (timerId_ == -1) {
        timerId_ = startTimer(5000); // Check every 5 seconds
    }
}

void SecurityService::stopMonitoring()
{
    if (timerId_ != -1) {
        killTimer(timerId_);
        timerId_ = -1;
    }
}

bool SecurityService::isMultipleDisplaysActive() const
{
    return QGuiApplication::screens().size() > 1;
}

void SecurityService::performCheck()
{
    if (isDebuggerAttached()) {
        emit secureViolationDetected("Debugger attached");
    }

    if (isMultipleDisplaysActive()) {
        emit secureViolationDetected("Multiple displays detected");
    }

    QStringList detected = detectProhibitedProcesses();
    if (!detected.isEmpty()) {
        emit secureViolationDetected(QString("Prohibited processes detected: %1").arg(detected.join(", ")));
    }
}

void SecurityService::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId_) {
        performCheck();
    }
}

} // namespace seb::security
