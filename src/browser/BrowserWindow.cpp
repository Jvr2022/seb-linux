#include "BrowserWindow.h"

#include "../browser_window.h"

namespace seb::browser {

BrowserWindowPort::BrowserWindowPort(::BrowserWindow *window, QObject *parent)
    : QObject(parent)
    , window_(window)
{
}

QString BrowserWindowPort::handle() const
{
    return window_ ? QString::number(reinterpret_cast<quintptr>(window_)) : QString();
}

QString BrowserWindowPort::iconPath() const
{
    return window_ ? window_->taskbarIconPath() : QString();
}

QString BrowserWindowPort::title() const
{
    return window_ ? window_->taskbarTitle() : QString();
}

void BrowserWindowPort::activate()
{
    if (!window_) {
        return;
    }

    window_->showNormal();
    window_->raise();
    window_->activateWindow();
}

bool BrowserWindowPort::isMainWindow() const
{
    return window_ ? window_->isMainWindow() : false;
}

QString BrowserWindowPort::url() const
{
    return window_ && window_->page() ? window_->page()->url().toString() : QString();
}

BrowserWindowContext BrowserWindowPort::context() const
{
    BrowserWindowContext context;
    if (window_) {
        context.id = static_cast<int>(reinterpret_cast<quintptr>(window_) & 0x7fffffff);
        context.isMainWindow = window_->isMainWindow();
        context.title = window_->taskbarTitle();
        context.url = window_->page() ? window_->page()->url().toString() : QString();
    }
    return context;
}

}  // namespace seb::browser
