#include "security_service.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QProcess>
#include <QTimerEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>
#include <QFileInfo>

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
    // Check TracerPid in /proc/self/status
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
    static const QStringList prohibited = {
        QStringLiteral("discord"), QStringLiteral("slack"), QStringLiteral("teamviewer"),
        QStringLiteral("anydesk"), QStringLiteral("obs"), QStringLiteral("simplescreenrecorder"),
        QStringLiteral("wireshark"), QStringLiteral("tcpdump"),
        QStringLiteral("strace")
    };

    QStringList detected;
    QDir proc(QStringLiteral("/proc"));
    const QStringList pids = proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &pid : pids) {
        bool ok;
        pid.toInt(&ok);
        if (!ok) continue;

        QFile cmdline(QStringLiteral("/proc/%1/cmdline").arg(pid));
        if (cmdline.open(QIODevice::ReadOnly)) {
            const QByteArray data = cmdline.readAll();
            if (data.isEmpty()) continue;

            const QString name = QString::fromLocal8Bit(data.constData()).section(QLatin1Char('\0'), 0, 0);
            const QString baseName = QFileInfo(name).fileName().toLower();

            for (const QString &p : prohibited) {
                if (baseName.contains(p)) {
                    detected << name;
                    break; // Move to next process immediately
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
