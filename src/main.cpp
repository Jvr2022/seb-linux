#include "app_controller.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QIcon>
#include <QTextStream>
#include <QProcess>
#include <cstdio>
#include <unistd.h>

namespace {

bool validateEngineEnvironment(int argc, char *argv[])
{
    if (argc == 2) {
        const QString argument = QString::fromLocal8Bit(argv[1]);
        if (argument == QStringLiteral("--help") || argument == QStringLiteral("--version")) return true;
    }
    if (geteuid() == 0) {
        fprintf(stderr, "Safe Exam Browser must run as an unprivileged user.\n");
        return false;
    }
    const QByteArray sandbox = qgetenv("QTWEBENGINE_DISABLE_SANDBOX").trimmed();
    if ((!sandbox.isEmpty() && sandbox != "0") || !qgetenv("QTWEBENGINE_REMOTE_DEBUGGING").isEmpty()) {
        fprintf(stderr, "Unsafe Qt WebEngine environment: sandboxing is disabled or remote debugging is enabled.\n");
        return false;
    }
    QStringList arguments = QProcess::splitCommand(qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS"));
    for (int i = 1; i < argc; ++i) arguments.append(QString::fromLocal8Bit(argv[i]));
    for (const auto &argument : arguments) {
        const QString flag = argument.section(QLatin1Char('='), 0, 0);
        if (QStringList{QStringLiteral("--no-sandbox"), QStringLiteral("--disable-web-security"),
            QStringLiteral("--ignore-certificate-errors"), QStringLiteral("--ignore-certificate-errors-spki-list"),
            QStringLiteral("--remote-debugging-port"), QStringLiteral("--remote-debugging-pipe"),
            QStringLiteral("--renderer-cmd-prefix"), QStringLiteral("--single-process"),
            QStringLiteral("--disable-site-isolation-trials"), QStringLiteral("--disable-seccomp-filter-sandbox"),
            QStringLiteral("--disable-namespace-sandbox"), QStringLiteral("--unsafely-treat-insecure-origin-as-secure"),
            QStringLiteral("--webEngineArgs")}.contains(flag)) {
            fprintf(stderr, "Unsafe Chromium startup argument rejected.\n");
            return false;
        }
    }
    return true;
}

}

int main(int argc, char *argv[])
{
    if (!validateEngineEnvironment(argc, argv)) return 1;

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

    parser.addPositionalArgument(
        QStringLiteral("resource"),
        QStringLiteral("Open a local .seb file, remote .seb URL, or seb:// / sebs:// resource."));

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

    return app.exec();
}
