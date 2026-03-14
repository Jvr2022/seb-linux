#include "application_window.h"

#include "../shell/taskbar/platform/command_helper.h"

namespace seb::applications {

ApplicationWindow::ApplicationWindow(QString windowId, QString title, QString iconPath, QObject *parent)
    : QObject(parent)
    , windowId_(std::move(windowId))
    , title_(std::move(title))
    , iconPath_(std::move(iconPath))
{
}

QString ApplicationWindow::iconPath() const
{
    return iconPath_;
}

QString ApplicationWindow::title() const
{
    return title_;
}

QString ApplicationWindow::handle() const
{
    return windowId_;
}

QString ApplicationWindow::windowId() const
{
    return windowId_;
}

void ApplicationWindow::activate()
{
    seb::shell::taskbar::platform::runCommand(QStringLiteral("xdotool"), {QStringLiteral("windowactivate"), windowId_});
}

void ApplicationWindow::update(const QString &title, const QString &iconPath)
{
    if (title_ == title && iconPath_ == iconPath) {
        return;
    }

    title_ = title;
    iconPath_ = iconPath;
    emit changed();
}

}  // namespace seb::applications
