#include "app_controller.h"

#include "browser_window.h"
#include "seb_session.h"
#include "ui/runtime_window.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <QTextStream>

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

AppController::~AppController() = default;

bool AppController::launch(const QString &resource, QString *error)
{
    seb::ResourceLoadResult loaded;
    if (resource.trimmed().isEmpty()) {
        loaded.ok = true;
        loaded.settings = seb::defaultSettings();
    } else {
        loaded = seb::loadSettingsFromResource(resource, [this](bool hashed) {
            bool accepted = false;
            const QString text = QInputDialog::getText(
                mainWindow_,
                hashed ? QStringLiteral("SEB Password Required") : QStringLiteral("SEB Password Required"),
                hashed
                    ? QStringLiteral("Enter the administrator password for this client-configuration SEB file.")
                    : QStringLiteral("Enter the password for this SEB configuration file."),
                QLineEdit::Password,
                QString(),
                &accepted);
            return accepted ? text : QString();
        });
    }

    if (!loaded.ok) {
        if (error) {
            *error = loaded.error;
        }
        return false;
    }

    return applySettings(loaded.settings, loaded.warnings, error);
}

bool AppController::launchResolved(const seb::SebSettings &settings, const QStringList &warnings, QString *error)
{
    return applySettings(settings, warnings, error);
}

bool AppController::applySettings(const seb::SebSettings &settings, const QStringList &warnings, QString *error)
{
    if (!settings.browser.enableBrowser) {
        if (error) {
            *error = QStringLiteral("The loaded configuration disables the browser.");
        }
        return false;
    }

    QTextStream err(stderr);
    for (const QString &warning : warnings) {
        err << "warning: " << warning << Qt::endl;
    }

    if (!runtimeWindow_) {
        runtimeWindow_ = std::make_unique<RuntimeWindow>();
    }
    runtimeWindow_->setVersionText(
        QCoreApplication::applicationVersion(),
        QCoreApplication::applicationVersion());
    runtimeWindow_->setStatusText(QStringLiteral("Starting Safe Exam Browser"));
    runtimeWindow_->show();
    runtimeWindow_->raise();
    runtimeWindow_->activateWindow();

    QElapsedTimer startupTimer;
    startupTimer.start();

    auto nextSession = std::make_unique<SebSession>(
        settings,
        [this](const QString &resource, QWidget *parent) {
            QString error;
            if (launch(resource, &error)) {
                return true;
            }

            QMessageBox::critical(parent, QStringLiteral("Failed to Open SEB Resource"), error);
            return false;
        },
        this);

    BrowserWindow *nextWindow = nextSession->createWindow(nextSession->initialUrl(), true);
    runtimeWindow_->setStatusText(QStringLiteral("Opening secure browser window"));

    const qint64 remaining = 5000 - startupTimer.elapsed();
    if (remaining > 0) {
        QEventLoop loop;
        QTimer::singleShot(static_cast<int>(remaining), &loop, &QEventLoop::quit);
        loop.exec();
    }

    nextWindow->show();
    runtimeWindow_->hide();

    if (mainWindow_) {
        mainWindow_->hide();
        mainWindow_->deleteLater();
    }

    mainWindow_ = nextWindow;
    session_ = std::move(nextSession);

    return true;
}
