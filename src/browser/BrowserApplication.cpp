#include "BrowserApplication.h"

#include "../browser_window.h"
#include "../seb_session.h"
#include "BrowserWindow.h"

namespace seb::browser {

BrowserApplication::BrowserApplication(SebSession &session, QObject *parent)
    : QObject(parent)
    , session_(session)
{
}

bool BrowserApplication::autoStart() const
{
    return true;
}

QString BrowserApplication::iconPath() const
{
    return QStringLiteral(":/assets/icons/safe-exam-browser.png");
}

QString BrowserApplication::id() const
{
    return QStringLiteral("safe-exam-browser");
}

QString BrowserApplication::name() const
{
    return QStringLiteral("Safe Exam Browser");
}

QString BrowserApplication::tooltip() const
{
    return name();
}

QList<seb::applications::contracts::IApplicationWindow *> BrowserApplication::windows() const
{
    QList<seb::applications::contracts::IApplicationWindow *> result;
    const auto ports = browserWindows();
    for (auto *window : ports) {
        result.push_back(window);
    }
    return result;
}

QList<contracts::IBrowserWindow *> BrowserApplication::browserWindows() const
{
    QList<contracts::IBrowserWindow *> result;
    for (auto *window : session_.browserWindows()) {
        if (window) {
            auto *port = windowPorts_.value(window, nullptr);
            if (!port) {
                auto *owner = const_cast<BrowserApplication *>(this);
                port = new BrowserWindowPort(window, owner);
                windowPorts_.insert(window, port);
                connect(window, &QObject::destroyed, owner, [this, window, port] {
                    windowPorts_.remove(window);
                    port->deleteLater();
                });
            }
            result.push_back(port);
        }
    }
    return result;
}

void BrowserApplication::focus(bool forward)
{
    const auto windows = session_.browserWindows();
    if (windows.isEmpty() || !windows.first()) {
        return;
    }

    int active = -1;
    for (int i = 0; i < windows.size(); ++i) if (windows.at(i)->isActiveWindow()) active = i;
    const int index = active < 0 ? 0 : (active + (forward ? 1 : windows.size() - 1)) % windows.size();
    auto *window = windows.at(index);
    if (window->isMinimized()) window->setWindowState(window->windowState() & ~Qt::WindowMinimized);
    window->show();
    window->raise();
    window->activateWindow();
    window->setFocus();
}

void BrowserApplication::initialize()
{
    context_.startUrl = session_.initialUrl().toString();
}

void BrowserApplication::start()
{
    if (session_.browserWindows().isEmpty()) {
        auto *window = session_.createWindow(session_.initialUrl(), true);
        window->show();
    }
}

void BrowserApplication::terminate()
{
    for (auto *window : session_.browserWindows()) {
        if (window) {
            window->close();
        }
    }
}

}  // namespace seb::browser
