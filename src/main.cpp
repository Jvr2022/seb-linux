#include "app_controller.h"
#include "security/security_service.h"
#include "browser/webengine_environment.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QMessageBox>
#include <QTextStream>
#include <QUrl>
#include <QProcess>
#include <QProcessEnvironment>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <cstdio>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <signal.h>
namespace {

int g_tty0_fd = -1;
int g_new_tty_fd = -1;
int g_original_vt = -1;

void cleanup_vt_and_exit() {
    if (g_new_tty_fd >= 0) {
        ioctl(g_new_tty_fd, KDSETMODE, KD_TEXT);
    }
    if (g_tty0_fd >= 0) {
        int target_vt = (g_original_vt > 0) ? g_original_vt : 1;
        ioctl(g_tty0_fd, VT_ACTIVATE, target_vt);
        ioctl(g_tty0_fd, VT_WAITACTIVE, target_vt);
    }
    if (g_new_tty_fd >= 0) close(g_new_tty_fd);
    if (g_tty0_fd >= 0) close(g_tty0_fd);
}

void barebones_sig_handler(int signum) {
    cleanup_vt_and_exit();
    if (signum == SIGSEGV || signum == SIGABRT || signum == SIGFPE || signum == SIGILL) {
        signal(signum, SIG_DFL);
        raise(signum);
    } else {
        _exit(1);
    }
}

bool setup_barebones_vt() {
    signal(SIGINT, barebones_sig_handler);
    signal(SIGTERM, barebones_sig_handler);
    signal(SIGSEGV, barebones_sig_handler);
    signal(SIGABRT, barebones_sig_handler);
    signal(SIGILL, barebones_sig_handler);
    signal(SIGFPE, barebones_sig_handler);

    g_tty0_fd = open("/dev/tty0", O_RDWR);
    if (g_tty0_fd < 0) {
        qWarning() << "Could not open /dev/tty0. Did you run the app as root?";
        return false;
    }

    int free_vt = -1;
    if (ioctl(g_tty0_fd, VT_OPENQRY, &free_vt) < 0 || free_vt == -1) {
        qWarning() << "Could not find a free VT.";
        close(g_tty0_fd);
        g_tty0_fd = -1;
        return false;
    }

    char vt_name[20];
    snprintf(vt_name, sizeof(vt_name), "/dev/tty%d", free_vt);
    g_new_tty_fd = open(vt_name, O_RDWR);
    if (g_new_tty_fd < 0) {
        qWarning() << "Could not open VT" << vt_name;
        close(g_tty0_fd);
        g_tty0_fd = -1;
        return false;
    }

    struct vt_stat vts;
    if (ioctl(g_tty0_fd, VT_GETSTATE, &vts) == 0) {
        g_original_vt = vts.v_active;
    }

    if (ioctl(g_tty0_fd, VT_ACTIVATE, free_vt) < 0 ||
        ioctl(g_tty0_fd, VT_WAITACTIVE, free_vt) < 0) {
        qWarning() << "Could not switch to VT" << free_vt;
        close(g_new_tty_fd);
        g_new_tty_fd = -1;
        close(g_tty0_fd);
        g_tty0_fd = -1;
        return false;
    }

    if (ioctl(g_new_tty_fd, KDSETMODE, KD_GRAPHICS) < 0) {
        qWarning() << "Error setting graphics mode on VT" << free_vt;
        int target_vt = (g_original_vt > 0) ? g_original_vt : 1;
        ioctl(g_tty0_fd, VT_ACTIVATE, target_vt);
        ioctl(g_tty0_fd, VT_WAITACTIVE, target_vt);
        close(g_new_tty_fd);
        g_new_tty_fd = -1;
        close(g_tty0_fd);
        g_tty0_fd = -1;
        return false;
    }

    atexit(cleanup_vt_and_exit);
    return true;
}

QString findConfigPath(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index) {
        const QString argument = QString::fromLocal8Bit(argv[index]);
        if ((argument == QStringLiteral("--config") || argument == QStringLiteral("-c")) && index + 1 < argc) {
            return QString::fromLocal8Bit(argv[index + 1]);
        }
        if (!argument.startsWith('-')) {
            return argument;
        }
    }
    return {};
}

bool hasArgument(int argc, char *argv[], const QString &value)
{
    for (int index = 1; index < argc; ++index) {
        if (QString::fromLocal8Bit(argv[index]) == value) {
            return true;
        }
    }
    return false;
}

QString xkbValueAfterColon(const QString &line, const QString &key)
{
    const QString trimmed = line.trimmed();
    if (!trimmed.startsWith(key)) {
        return {};
    }

    const int separator = trimmed.indexOf(QLatin1Char(':'));
    if (separator < 0) {
        return {};
    }

    return trimmed.mid(separator + 1).trimmed();
}

QString stripQuotes(QString value)
{
    value = value.trimmed();
    if (value.size() >= 2 && value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"'))) {
        return value.mid(1, value.size() - 2);
    }
    return value;
}

bool isUsableKeyboardValue(const QString &value)
{
    return !value.isEmpty() && value != QStringLiteral("(unset)") && value != QStringLiteral("n/a");
}

// The barebones VT uses Qt's "linuxfb" platform, whose keyboard handlers
// (libinput/xkbcommon, or the evdev fallback) never read the layout configured
// for the desktop session. Without these variables xkbcommon defaults to "us",
// which is why an azerty session ends up with a qwerty keyboard. Propagate the
// configured layout so the exam shell keeps the expected key mapping.
void applyKeyboardLayoutEnvironment()
{
    if (!qgetenv("XKB_DEFAULT_LAYOUT").isEmpty()) {
        return;
    }

    QString layout;
    QString variant;
    QString model;
    QString options;

    QProcess localectl;
    localectl.start(QStringLiteral("localectl"), {QStringLiteral("status")});
    if (localectl.waitForFinished(3000)) {
        const QString output = QString::fromUtf8(localectl.readAllStandardOutput());
        const QStringList lines = output.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &line : lines) {
            const auto takeValue = [&line](QString &target, const QString &key) {
                if (target.isEmpty()) {
                    const QString value = xkbValueAfterColon(line, key);
                    if (isUsableKeyboardValue(value)) {
                        target = value;
                    }
                }
            };
            takeValue(layout, QStringLiteral("X11 Layout"));
            takeValue(variant, QStringLiteral("X11 Variant"));
            takeValue(model, QStringLiteral("X11 Model"));
            takeValue(options, QStringLiteral("X11 Options"));
        }
    }

    if (layout.isEmpty()) {
        QFile keyboardConfig(QStringLiteral("/etc/default/keyboard"));
        if (keyboardConfig.open(QIODevice::ReadOnly | QIODevice::Text)) {
            const QString content = QString::fromUtf8(keyboardConfig.readAll());
            const QStringList lines = content.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            for (const QString &line : lines) {
                if (line.trimmed().startsWith(QLatin1Char('#'))) {
                    continue;
                }

                const int separator = line.indexOf(QLatin1Char('='));
                if (separator < 0) {
                    continue;
                }

                const QString key = line.left(separator).trimmed();
                const QString value = stripQuotes(line.mid(separator + 1));
                if (key == QStringLiteral("XKBLAYOUT")) {
                    layout = value;
                } else if (key == QStringLiteral("XKBVARIANT")) {
                    variant = value;
                } else if (key == QStringLiteral("XKBMODEL")) {
                    model = value;
                } else if (key == QStringLiteral("XKBOPTIONS")) {
                    options = value;
                }
            }
        }
    }

    if (!isUsableKeyboardValue(layout)) {
        return;
    }

    qputenv("XKB_DEFAULT_LAYOUT", layout.toUtf8());
    if (isUsableKeyboardValue(variant)) {
        qputenv("XKB_DEFAULT_VARIANT", variant.toUtf8());
    }
    if (isUsableKeyboardValue(model)) {
        qputenv("XKB_DEFAULT_MODEL", model.toUtf8());
    }
    if (isUsableKeyboardValue(options)) {
        qputenv("XKB_DEFAULT_OPTIONS", options.toUtf8());
    }
}

void applyEarlyEnvironment(int argc, char *argv[])
{
    const QString configPath = findConfigPath(argc, argv);
    const bool hasResource = !configPath.isEmpty();
    const bool isExamAntiCheat = hasArgument(argc, argv, QStringLiteral("--anti-cheat"));
    const bool isMenuLockdown = hasArgument(argc, argv, QStringLiteral("--menu-lockdown"));
    if (isExamAntiCheat || isMenuLockdown) {
        if (isMenuLockdown || (isExamAntiCheat && hasResource)) {
            if (!setup_barebones_vt()) {
                qCritical() << "Anti-cheat VT setup failed; aborting.";
                _exit(1);
            }
            applyKeyboardLayoutEnvironment();
            qputenv("QT_QPA_PLATFORM", "linuxfb");
            qputenv("QT_QUICK_BACKEND", "software");
        }
#if SEB_HAS_QTWEBENGINE
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif
    }

}

}  // namespace

int main(int argc, char *argv[])
{
    applyEarlyEnvironment(argc, argv);

    QApplication app(argc, argv);
    
     const QIcon appIcon(QStringLiteral(":/assets/icons/safe-exam-browser.png"));
    app.setWindowIcon(appIcon);
    app.setDesktopFileName(QStringLiteral("safe-exam-browser"));
    QCoreApplication::setApplicationName(QStringLiteral("Safe Exam Browser"));
    QCoreApplication::setApplicationVersion(QStringLiteral(SEB_APPLICATION_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
        "Safe Exam Browser for Linux with .seb file support, exam link handling, and Qt WebEngine or WebKitGTK browser backends."));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("c"), QStringLiteral("config")},
        QStringLiteral("Load settings from a JSON file or an unencrypted XML plist .seb file."),
        QStringLiteral("file")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("u"), QStringLiteral("url")},
        QStringLiteral("Override the configured start URL."),
        QStringLiteral("url")));
    parser.addPositionalArgument(
        QStringLiteral("resource"),
        QStringLiteral("Open a local .seb file, remote .seb URL, or seb:// / sebs:// resource."));
    parser.addOption(QCommandLineOption(QStringLiteral("show-toolbar"), QStringLiteral("Show the browser toolbar.")));
    parser.addOption(QCommandLineOption(QStringLiteral("allow-address-bar"), QStringLiteral("Enable the address bar.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("allow-navigation"),
        QStringLiteral("Enable back and forward navigation in the main window.")));
    parser.addOption(QCommandLineOption(QStringLiteral("allow-reload"), QStringLiteral("Enable reload in the main window.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("allow-devtools"),
        QStringLiteral("Enable the developer tools shortcut (F12).")));
    parser.addOption(QCommandLineOption(QStringLiteral("windowed"), QStringLiteral("Force the main window to stay windowed.")));
    parser.addOption(QCommandLineOption(QStringLiteral("fullscreen"), QStringLiteral("Force the main window to be fullscreen.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("disable-quit"),
        QStringLiteral("Disable manual termination even if the configuration allows it.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("anti-cheat")},
        QStringLiteral("Enable anticheat mode.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("menu-lockdown")},
        QStringLiteral("Enable the protected start-menu lockdown mode.")));
#if defined(QT_DEBUG) || defined(SEB_DEV_BYPASS_OPTION)
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("dev-bypass")},
        QStringLiteral("Skip strict lockdowns for development purposes.")));
#endif

    parser.process(app);

    QTextStream err(stderr);

#if !SEB_HAS_QTWEBENGINE
    err << "warning: This build was compiled without QtWebEngine support. "
           "Safe Exam Browser will start in compatibility mode and cannot render exam pages."
        << Qt::endl;
#endif

    const QString resource = parser.isSet("config")
        ? parser.value("config")
        : (parser.positionalArguments().isEmpty() ? QString() : parser.positionalArguments().constFirst());

    AppController controller;
    QString launchError;
    if (!controller.launch(resource, &launchError)) {
        if (!launchError.isEmpty()) {
            err << launchError << Qt::endl;
        }
        return launchError == QStringLiteral("Client configuration saved successfully.") ? 0 : 1;
    }

    bool devBypass = false;
#if defined(QT_DEBUG) || defined(SEB_DEV_BYPASS_OPTION)
    devBypass = parser.isSet("dev-bypass");
#endif
#ifdef SEB_DEV_BYPASS_DEFAULT
    devBypass = true;
#endif

    seb::security::SecurityService security;
    if (!devBypass) {
        if (security.isVirtualMachine()) {
            err << "Error: Running in a virtual machine is not allowed." << Qt::endl;
            return 1;
        }
        if (security.isDebuggerAttached()) {
            err << "Error: A debugger is attached." << Qt::endl;
            return 1;
        }

        QObject::connect(&security, &seb::security::SecurityService::secureViolationDetected, [&app](const QString &reason) {
            qCritical() << "Security Violation:" << reason;
            app.quit();
        });
        security.startMonitoring();
    } else {
        qDebug() << "Developer bypass active; security monitoring disabled.";
    }

    return app.exec();
}
