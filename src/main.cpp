#include "app_controller.h"
#include "browser/webengine_environment.h"
#include "seb_settings.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
#include <QTextStream>
#include <QUrl>
#include <QFileInfo>

namespace {

QString findConfigPath(int argc, char *argv[])
{
    for (int index = 1; index < argc; ++index) {
        const QString argument = QString::fromLocal8Bit(argv[index]);
        if ((argument == QStringLiteral("--config") || argument == QStringLiteral("-c")) && index + 1 < argc) {
            return QString::fromLocal8Bit(argv[index + 1]);
        }
        if (argument == QStringLiteral("--url") || argument == QStringLiteral("-u") ||
            argument == QStringLiteral("--inject") || argument == QStringLiteral("-i")) {
            ++index;
            continue;
        }
        if (!argument.startsWith('-')) {
            return argument;
        }
    }
    return {};
}

void applyEarlyEnvironment(int argc, char *argv[])
{
    seb::SebSettings settings = seb::defaultSettings();
    const QString configPath = findConfigPath(argc, argv);
    if (!configPath.isEmpty()) {
        const seb::LoadResult loaded = seb::loadSettingsFromFile(configPath);
        if (loaded.ok) {
            settings = loaded.settings;
        }
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

    if (parser.isSet("always-on-top")) {
        settings.browser.mainWindow.alwaysOnTop = true;
    }

    if (parser.isSet("disable-minimize")) {
        settings.browser.mainWindow.allowMinimize = false;
    }

    if (parser.isSet("disable-quit")) {
        settings.security.allowTermination = false;
    }

    if (parser.isSet("inject")) {
        settings.browser.injectedScript = QFileInfo(parser.value("inject")).absoluteFilePath();
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
    parser.addOption(QCommandLineOption(QStringLiteral("always-on-top"), QStringLiteral("Keep the main window above other windows.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("disable-minimize"),
        QStringLiteral("Prevent minimizing the main exam window.")));
    parser.addOption(QCommandLineOption(
        QStringLiteral("disable-quit"),
        QStringLiteral("Disable manual termination even if the configuration allows it.")));
    parser.addOption(QCommandLineOption(
        QStringList{QStringLiteral("i"), QStringLiteral("inject")},
        QStringLiteral("Inject a JavaScript file into each page."),
        QStringLiteral("file")));

    parser.process(app);

    seb::SebSettings settings = seb::defaultSettings();
    QTextStream err(stderr);

    const QString resource = parser.isSet("config")
        ? parser.value("config")
        : (parser.positionalArguments().isEmpty() ? QString() : parser.positionalArguments().constFirst());

    QStringList warnings;
    if (!resource.isEmpty()) {
        const seb::ResourceLoadResult loaded = seb::loadSettingsFromResource(resource, [](bool hashed) {
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

    AppController controller;
    QString launchError;
    if (!controller.launchResolved(settings, warnings, &launchError)) {
        err << launchError << Qt::endl;
        return 1;
    }

    return app.exec();
}
