#include "BrowserControl.h"
#include "contracts/i_webview.h"

namespace seb::browser {

BrowserControl::BrowserControl(contracts::IWebView *view, QObject *parent)
    : QObject(parent)
    , view_(view)
{
}


QString BrowserControl::address() const
{
#if SEB_HAS_QTWEBENGINE
    return view_ ? view_->url().toString() : QString();
#else
    return {};
#endif
}

bool BrowserControl::canNavigateBackwards() const
{
    return view_ && view_->canGoBack();
}

bool BrowserControl::canNavigateForwards() const
{
    return view_ && view_->canGoForward();
}

void BrowserControl::navigateTo(const QString &address)
{
#if SEB_HAS_QTWEBENGINE
    if (view_) {
        view_->setUrl(QUrl::fromUserInput(address));
    }
#else
    Q_UNUSED(address);
#endif
}

void BrowserControl::navigateBackwards()
{
    if (view_) {
        view_->back();
    }
}

void BrowserControl::navigateForwards()
{
    if (view_) {
        view_->forward();
    }
}

void BrowserControl::reload()
{
    if (view_) {
        view_->reload();
    }
}

}  // namespace seb::browser
