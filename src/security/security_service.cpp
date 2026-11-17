#include "security_service.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QThread>
#include <unistd.h>

namespace seb::security {
namespace {
QString readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()).trimmed() : QString();
}

bool internalDisplay(const QString &name)
{
    return name.startsWith(QStringLiteral("eDP"), Qt::CaseInsensitive) ||
        name.startsWith(QStringLiteral("LVDS"), Qt::CaseInsensitive) ||
        name.startsWith(QStringLiteral("DSI"), Qt::CaseInsensitive);
}

bool reject(QString *error, const QString &message)
{
    if (error) *error = message;
    return false;
}
}

SecurityService::SecurityService(QObject *parent) : QObject(parent)
{
    connect(qGuiApp, &QGuiApplication::screenAdded, this, [this] { if (timerId_ != -1) performCheck(); });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [this] { if (timerId_ != -1) performCheck(); });
}

bool SecurityService::isVirtualMachine() const
{
    const QString cpu = readFile(QStringLiteral("/proc/cpuinfo"));
    if (QRegularExpression(QStringLiteral("\\bhypervisor\\b"), QRegularExpression::CaseInsensitiveOption).match(cpu).hasMatch()) return true;
    const QString hardware = readFile(QStringLiteral("/sys/class/dmi/id/sys_vendor")) + QLatin1Char(' ') +
        readFile(QStringLiteral("/sys/class/dmi/id/product_name"));
    for (const auto &vendor : {"VMware", "VirtualBox", "QEMU", "KVM", "Bochs", "Xen", "Parallels", "Virtual Machine"})
        if (hardware.contains(QString::fromLatin1(vendor), Qt::CaseInsensitive)) return true;
    return false;
}

bool SecurityService::isDebuggerAttached() const
{
    const QString status = readFile(QStringLiteral("/proc/self/status"));
    const auto match = QRegularExpression(QStringLiteral("(?:^|\\n)TracerPid:\\s*(\\d+)")).match(status);
    return match.hasMatch() && match.captured(1).toULongLong() != 0;
}

bool SecurityService::validateDisplay(const seb::settings::monitoring::DisplaySettings &settings, QString *error) const
{
    if (settings.AllowedDisplays < 1) return reject(error, QStringLiteral("The allowed display count must be positive."));
    int count = 0;
    int external = 0;
    const QDir drm(QStringLiteral("/sys/class/drm"));
    for (const auto &connector : drm.entryList({QStringLiteral("card*-*")}, QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (readFile(drm.filePath(connector + QStringLiteral("/status"))) != QStringLiteral("connected") ||
            readFile(drm.filePath(connector + QStringLiteral("/enabled"))) != QStringLiteral("enabled")) continue;
        ++count;
        const QString name = connector.mid(connector.indexOf(QLatin1Char('-')) + 1);
        if (!internalDisplay(name)) ++external;
    }
    const auto screens = QGuiApplication::screens();
    if (count == 0) {
        for (const auto *screen : screens) {
            ++count;
            if (!internalDisplay(screen->name())) ++external;
        }
    } else if (screens.size() > count) {
        external += screens.size() - count;
        count = screens.size();
    }
    if (count == 0) return settings.IgnoreError || reject(error, QStringLiteral("Unable to validate the display configuration."));
    if (count > settings.AllowedDisplays)
        return reject(error, QStringLiteral("Detected %1 active displays; this exam allows %2.").arg(count).arg(settings.AllowedDisplays));
    if (settings.InternalDisplayOnly && external > 0)
        return reject(error, QStringLiteral("This exam permits internal displays only."));
    return true;
}

QStringList SecurityService::detectProhibitedProcesses(const seb::settings::applications::ApplicationSettings &settings) const
{
    QStringList prohibited;
    for (const auto &rule : settings.Blacklist)
        if (!rule.ExecutableName.isEmpty()) prohibited.append(QFileInfo(rule.ExecutableName).fileName());
    if (prohibited.isEmpty()) return {};
    QStringList detected;
    const QDir proc(QStringLiteral("/proc"));
    for (const auto &pid : proc.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        bool valid = false;
        const auto number = pid.toLongLong(&valid);
        if (!valid || number <= 0 || number == getpid()) continue;
        const QString executable = QFileInfo(QStringLiteral("/proc/%1/exe").arg(pid)).symLinkTarget();
        if (executable.isEmpty()) continue;
        QString path = executable;
        if (path.endsWith(QStringLiteral(" (deleted)")) && !QFileInfo::exists(path)) path.chop(10);
        const QString name = QFileInfo(path).fileName();
        if (prohibited.contains(name, Qt::CaseInsensitive) && !detected.contains(name)) detected.append(name);
    }
    return detected;
}

bool SecurityService::validateSettings(const seb::settings::AppSettings &settings, QString *error) const
{
    if (isDebuggerAttached()) return reject(error, QStringLiteral("A debugger is attached to Safe Exam Browser."));
    if (settings.Security.VirtualMachinePolicy == seb::settings::security::VirtualMachinePolicy::Deny && isVirtualMachine())
        return reject(error, QStringLiteral("This exam does not permit virtual machines."));
    if (settings.Security.VerifyCursorConfiguration)
        return reject(error, QStringLiteral("This exam requires cursor integrity verification, which is unavailable on Linux."));
    if (!settings.Service.IgnoreService && settings.Service.Policy == seb::settings::service::ServicePolicy::Mandatory)
        return reject(error, QStringLiteral("This exam requires a privileged SEB service, which is unavailable on Linux."));
    if (!settings.Service.IgnoreService && settings.Service.Policy == seb::settings::service::ServicePolicy::Optional &&
        (settings.Service.DisableUserLock || settings.Service.DisableUserSwitch || settings.Service.DisableChromeNotifications ||
         settings.Service.DisableEaseOfAccessOptions || settings.Service.DisableFindPrinter || settings.Service.DisableNetworkOptions ||
         settings.Service.DisablePasswordChange || settings.Service.DisablePowerOptions || settings.Service.DisableRemoteConnections ||
         settings.Service.DisableSignout || settings.Service.DisableTaskManager || settings.Service.DisableVmwareOverlay ||
         settings.Service.DisableWindowsUpdate || settings.Service.SetVmwareConfiguration))
        return reject(error, QStringLiteral("This exam requests privileged system restrictions that the Linux client cannot apply or restore."));
    if (settings.Security.KioskMode != seb::settings::security::KioskMode::None)
        return reject(error, QStringLiteral("This exam requires Windows kiosk mode. Linux cannot create an isolated desktop or disable and restore the desktop shell, so the exam cannot start securely."));
    if (!settings.Security.AllowWindowCapture)
        return reject(error, QStringLiteral("This exam prohibits screen capture or sharing. Linux desktop environments do not provide a portable SEB control to enforce this setting."));
    if (settings.Security.DisableSessionChangeLockScreen)
        return reject(error, QStringLiteral("This exam requires control of the session-change lock screen, which is unavailable on Linux."));
    if ((settings.Display.AlwaysOn || settings.System.AlwaysOn) &&
        QStandardPaths::findExecutable(QStringLiteral("systemd-inhibit")).isEmpty())
        return reject(error, QStringLiteral("This exam requires the display and system to stay awake, but systemd-inhibit is unavailable."));
    if (!validateDisplay(settings.Display, error)) return false;
    if (!detectProhibitedProcesses(settings.Applications).isEmpty())
        return reject(error, QStringLiteral("Prohibited applications are running. Close them before starting this exam."));
    return true;
}

bool SecurityService::configure(const seb::settings::AppSettings &settings, QString *error)
{
    stopMonitoring();
    display_ = settings.Display;
    security_ = settings.Security;
    applications_ = settings.Applications;
    alwaysOnRequired_ = display_.AlwaysOn || settings.System.AlwaysOn;
    violationReported_ = false;
    if (alwaysOnRequired_) {
        const QString executable = QStandardPaths::findExecutable(QStringLiteral("systemd-inhibit"));
        if (executable.isEmpty()) return reject(error, QStringLiteral("Unable to keep the Linux session awake because systemd-inhibit is unavailable."));
        idleInhibitor_.start(executable, {
            QStringLiteral("--what=idle:sleep"), QStringLiteral("--mode=block"),
            QStringLiteral("--who=Safe Exam Browser"), QStringLiteral("--why=Exam session"),
            QStringLiteral("sleep"), QStringLiteral("infinity")});
        if (!idleInhibitor_.waitForStarted(2000))
            return reject(error, QStringLiteral("Unable to activate the Linux idle and sleep inhibitor: %1").arg(idleInhibitor_.errorString()));
        QThread::msleep(100);
        if (idleInhibitor_.state() != QProcess::Running)
            return reject(error, QStringLiteral("The Linux idle and sleep inhibitor exited before the exam started."));
    }
    return true;
}

void SecurityService::startMonitoring()
{
    if (timerId_ == -1) timerId_ = startTimer(5000);
}

void SecurityService::stopMonitoring()
{
    if (timerId_ != -1) { killTimer(timerId_); timerId_ = -1; }
    if (idleInhibitor_.state() != QProcess::NotRunning) {
        idleInhibitor_.terminate();
        if (!idleInhibitor_.waitForFinished(1000)) {
            idleInhibitor_.kill();
            idleInhibitor_.waitForFinished(1000);
        }
    }
}

void SecurityService::performCheck()
{
    if (violationReported_) return;
    QString error;
    if (alwaysOnRequired_ && idleInhibitor_.state() != QProcess::Running)
        error = QStringLiteral("The Linux idle and sleep inhibitor stopped during the exam.");
    else if (isDebuggerAttached()) error = QStringLiteral("A debugger was attached to Safe Exam Browser.");
    else if (security_.VirtualMachinePolicy == seb::settings::security::VirtualMachinePolicy::Deny && isVirtualMachine())
        error = QStringLiteral("Virtual machine detected during an exam that forbids it.");
    else if (!validateDisplay(display_, &error)) {
        if (error.isEmpty()) error = QStringLiteral("Display configuration validation failed.");
    }
    else if (!detectProhibitedProcesses(applications_).isEmpty()) error = QStringLiteral("A prohibited application was started.");
    if (!error.isEmpty()) {
        violationReported_ = true;
        emit secureViolationDetected(error);
    }
}

void SecurityService::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == timerId_) performCheck();
}

}
