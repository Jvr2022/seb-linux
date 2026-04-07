#include "app_controller.h"
#include "security/security_service.h"
#include "browser/webengine_environment.h"
#include "seb_settings.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTextStream>
#include <QUrl>
#include <QProcess>
#include <QProcessEnvironment>
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

static void appendPkexecEnvironmentVariable(QStringList &args, const char *name)
{
    const QByteArray value = qgetenv(name);
    if (!value.isEmpty()) {
        args << (QString(name) + QLatin1Char('=') + QString::fromLocal8Bit(value));
    }
}

void applyProtectedWindowSettings(seb::WindowSettings &settings, bool fullScreen)
{
    settings.absoluteHeight = 0;
    settings.absoluteWidth = 0;
    settings.relativeHeight = 100;
    settings.relativeWidth = 100;
    settings.allowAddressBar = false;
    settings.allowBackwardNavigation = false;
    settings.allowDeveloperConsole = false;
    settings.allowForwardNavigation = false;
    settings.allowMinimize = false;
    settings.allowReloading = false;
    settings.alwaysOnTop = true;
    settings.frameless = true;
    settings.fullScreenMode = fullScreen;
    settings.showHomeButton = false;
    settings.showReloadButton = false;
    settings.showReloadWarning = false;
    settings.showToolbar = false;
    settings.position = seb::WindowPosition::Center;
}

void applyProtectedSessionSettings(
    seb::SebSettings &settings,
    bool fullScreen,
    bool allowConfiguredApps,
    bool allowTermination)
{
    applyProtectedWindowSettings(settings.browser.mainWindow, fullScreen);
    applyProtectedWindowSettings(settings.browser.additionalWindow, false);
    settings.browser.additionalWindow.relativeWidth = 100;
    settings.browser.popupPolicy = seb::PopupPolicy::AllowSameWindow;
    settings.browser.allowConfigurationDownloads = false;
    settings.browser.allowCustomDownAndUploadLocation = false;
    settings.browser.allowDownloads = false;
    settings.browser.allowFind = false;
    settings.browser.allowPageZoom = false;
    settings.browser.allowPrint = false;
    settings.browser.allowSpellChecking = false;
    settings.browser.allowUploads = false;
    settings.taskbar.showApplicationInfo = false;
    settings.taskbar.showApplicationLog = false;
    settings.taskbar.showAudio = false;
    settings.taskbar.showKeyboardLayout = false;
    settings.taskbar.showNetwork = false;
    settings.security.allowTermination = allowTermination;

    if (!allowConfiguredApps) {
        settings.applications.whitelist.clear();
        settings.applications.blacklist.clear();
        settings.taskbar.showProctoringNotification = false;
    }
}

void applyEarlyEnvironment(int argc, char *argv[])
{
    seb::SebSettings settings = seb::defaultSettings();
    const QString configPath = findConfigPath(argc, argv);
    const bool hasResource = !configPath.isEmpty();
    const bool isExamAntiCheat = hasArgument(argc, argv, QStringLiteral("--anti-cheat"));
    const bool isMenuLockdown = hasArgument(argc, argv, QStringLiteral("--menu-lockdown"));
    if (!configPath.isEmpty()) {
        const seb::LoadResult loaded = seb::loadSettingsFromFile(configPath);
        if (loaded.ok) {
            settings = loaded.settings;
        }
    }

    if (isExamAntiCheat || isMenuLockdown) {
        if (isMenuLockdown || (isExamAntiCheat && hasResource)) {
            if (!setup_barebones_vt()) {
                qCritical() << "Anti-cheat VT setup failed; aborting.";
                _exit(1);
            }
            qputenv("QT_QPA_PLATFORM", "linuxfb");
            qputenv("QT_QUICK_BACKEND", "software");
        }
#if SEB_HAS_QTWEBENGINE
        qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--no-sandbox");
#endif
    }

    seb::browser::applyWebEngineEnvironment(settings);
}

void applyCommandLineOverrides(const QCommandLineParser &parser, seb::SebSettings &settings)
{
    if (parser.isSet("url")) {
        settings.browser.startUrl = parser.value("url").trimmed();
    }

    if (parser.isSet("show-toolbar")) {
        settings.browser.mainWindow.showToolbar = true;
    }

    if (parser.isSet("allow-address-bar")) {
        settings.browser.mainWindow.allowAddressBar = true;
        settings.browser.mainWindow.showToolbar = true;
    }

    if (parser.isSet("allow-navigation")) {
        settings.browser.mainWindow.allowBackwardNavigation = true;
        settings.browser.mainWindow.allowForwardNavigation = true;
    }

    if (parser.isSet("allow-reload")) {
        settings.browser.mainWindow.allowReloading = true;
        settings.browser.mainWindow.showReloadButton = true;
    }

    if (parser.isSet("allow-devtools")) {
        settings.browser.mainWindow.allowDeveloperConsole = true;
    }

    if (parser.isSet("windowed")) {
        settings.browser.mainWindow.fullScreenMode = false;
    }
    if (parser.isSet("fullscreen")){
        settings.browser.mainWindow.fullScreenMode = true;
    }
    if (parser.isSet("always-on-top")) {
        settings.browser.mainWindow.alwaysOnTop = true;
    }

    if (parser.isSet("disable-minimize")) {
        settings.browser.mainWindow.allowMinimize = false;
    }

    if (parser.isSet("disable-quit")) {
        settings.security.allowTermination = false;
    }

    if (parser.isSet("dev-bypass")) {
        settings.devBypass = true;
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
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral(
        "Expanded Linux port of Safe Exam Browser focused on portable browser and session features."));
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
    parser.addOption(QCommandLineOption(QStringLiteral("always-on-top"), QStringLiteral("Keep the main window above other windows.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("disable-minimize"),
        QStringLiteral("Prevent minimizing the main exam window.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("disable-quit"),
        QStringLiteral("Disable manual termination even if the configuration allows it.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("anti-cheat")},
        QStringLiteral("Enable anticheat mode.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("menu-lockdown")},
        QStringLiteral("Enable the protected start-menu lockdown mode.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("dev-bypass")},
        QStringLiteral("Skip strict lockdowns for development purposes.")));

    parser.process(app);

    seb::SebSettings settings = seb::defaultSettings();
    QTextStream err(stderr);

#if !SEB_HAS_QTWEBENGINE
    err << "warning: This build was compiled without QtWebEngine support. "
           "Safe Exam Browser will start in compatibility mode and cannot render exam pages."
        << Qt::endl;
#endif

    const QString resource = parser.isSet("config")
        ? parser.value("config")
        : (parser.positionalArguments().isEmpty() ? QString() : parser.positionalArguments().constFirst());

    QString userPassword;
    bool usedPassword = false;

    QStringList warnings;
    if (!resource.isEmpty()) {
        const seb::ResourceLoadResult loaded = seb::loadSettingsFromResource(resource, [&userPassword, &usedPassword](bool hashed) {
            if (qEnvironmentVariableIsSet("SEB_PASSWORD")) {
                userPassword = QString::fromUtf8(qgetenv("SEB_PASSWORD"));
                usedPassword = true;
                return userPassword;
            }

            bool accepted = false;
            const QString password = QInputDialog::getText(
                nullptr,
                QStringLiteral("SEB Password Required"),
                hashed
                    ? QStringLiteral("Enter the administrator password for this client-configuration SEB file.")
                    : QStringLiteral("Enter the password for this SEB configuration file."),
                QLineEdit::Password,
                QString(),
                &accepted);
            if (accepted) {
                userPassword = password;
                usedPassword = true;
            }
            return accepted ? password : QString();
        });
        if (!loaded.ok) {
            err << loaded.error << Qt::endl;
            return 1;
        }

        settings = loaded.settings;
        warnings = loaded.warnings;
    }

    applyCommandLineOverrides(parser, settings);

    const bool launchedWithoutExam = resource.isEmpty();
    const bool menuLockdown = parser.isSet("menu-lockdown");
    const bool examAntiCheat = parser.isSet("anti-cheat");
    bool devBypass = settings.devBypass || parser.isSet("dev-bypass");
#ifdef SEB_DEV_BYPASS_DEFAULT
    devBypass = true;
#endif
    
    if (!devBypass && !parser.isSet("anti-cheat") && !menuLockdown) {
        if (launchedWithoutExam) {
            const auto answer = QMessageBox::question(
                nullptr,
                QStringLiteral("Administrator Permission Required"),
                QStringLiteral(
                    "To continue using Safe Exam Browser from the start menu, administrator permission is required "
                    "so the app can apply lockdown protections.\n\nDo you want to continue?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Yes);
            if (answer != QMessageBox::Yes) {
                return 0;
            }

            QStringList args = QCoreApplication::arguments();
            args.removeFirst();
            args.prepend(QStringLiteral("--menu-lockdown"));

            QProcess child;
            child.setProgram(QStringLiteral("pkexec"));

            QStringList pkexecArgs;
            pkexecArgs << QStringLiteral("--keep-cwd");
            pkexecArgs << QStringLiteral("env");
            appendPkexecEnvironmentVariable(pkexecArgs, "DISPLAY");
            appendPkexecEnvironmentVariable(pkexecArgs, "WAYLAND_DISPLAY");
            appendPkexecEnvironmentVariable(pkexecArgs, "XAUTHORITY");
            appendPkexecEnvironmentVariable(pkexecArgs, "XDG_RUNTIME_DIR");
            appendPkexecEnvironmentVariable(pkexecArgs, "XDG_SESSION_TYPE");
            appendPkexecEnvironmentVariable(pkexecArgs, "DBUS_SESSION_BUS_ADDRESS");
            appendPkexecEnvironmentVariable(pkexecArgs, "QT_QPA_PLATFORM");
            pkexecArgs << QCoreApplication::applicationFilePath();
            pkexecArgs << args;

            child.setArguments(pkexecArgs);
            if (child.startDetached()) {
                return 0;
            }
            return 1;
        }

        const bool requiresLockedExamShell =
            settings.browser.mainWindow.fullScreenMode && settings.browser.mainWindow.alwaysOnTop;
        if (requiresLockedExamShell) {
            const auto answer = QMessageBox::question(
                nullptr,
                QStringLiteral("Administrator Permission Required"),
                QStringLiteral(
                    "To continue using Safe Exam Browser, administrator permission is required so the app can "
                    "apply exam locking and anti-cheat protections.\n\nDo you want to continue?"),
                QMessageBox::Yes | QMessageBox::Cancel,
                QMessageBox::Yes);
            if (answer != QMessageBox::Yes) {
                return 0;
            }

            QStringList args = QCoreApplication::arguments();
            args.removeFirst(); // Remove executable path
            if (!args.contains(QStringLiteral("--anti-cheat"))) {
                args.prepend(QStringLiteral("--anti-cheat"));
            }
            
            QProcess child;
            child.setProgram(QStringLiteral("pkexec"));
            
            QStringList pkexecArgs;
            pkexecArgs << QStringLiteral("--keep-cwd");
            pkexecArgs << QStringLiteral("env");
            appendPkexecEnvironmentVariable(pkexecArgs, "DISPLAY");
            appendPkexecEnvironmentVariable(pkexecArgs, "WAYLAND_DISPLAY");
            appendPkexecEnvironmentVariable(pkexecArgs, "XAUTHORITY");
            appendPkexecEnvironmentVariable(pkexecArgs, "XDG_RUNTIME_DIR");
            appendPkexecEnvironmentVariable(pkexecArgs, "XDG_SESSION_TYPE");
            appendPkexecEnvironmentVariable(pkexecArgs, "DBUS_SESSION_BUS_ADDRESS");
            appendPkexecEnvironmentVariable(pkexecArgs, "QT_QPA_PLATFORM");
            if (usedPassword) {
                pkexecArgs << (QStringLiteral("SEB_PASSWORD=") + userPassword);
            }
            pkexecArgs << QCoreApplication::applicationFilePath();
            pkexecArgs << args;
            
            child.setArguments(pkexecArgs);
            
            if (child.startDetached()) {
                return 0;
            }
            return 1;
        }
    }

    if (menuLockdown && launchedWithoutExam && !devBypass) {
        applyProtectedSessionSettings(settings, false, false, true);
    } else if (examAntiCheat && !devBypass) {
        applyProtectedSessionSettings(settings, true, true, false);
    } else if (!launchedWithoutExam &&
               settings.browser.mainWindow.fullScreenMode &&
               settings.browser.mainWindow.alwaysOnTop &&
               !devBypass) {
        applyProtectedSessionSettings(settings, true, true, false);
    }

    if (devBypass) {
        seb::applyDevBypassOverrides(settings);
    }

    AppController controller;
    QString launchError;
    if (!controller.launchResolved(settings, warnings, &launchError)) {
        err << launchError << Qt::endl;
        return 1;
    }

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
