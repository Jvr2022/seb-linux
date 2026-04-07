#include "BrowserControl.h"

#if SEB_HAS_QTWEBENGINE
#include <QWebEngineHistory>
#include <QWebEngineView>
#else
#include <QUrl>
#endif

namespace seb::browser {

BrowserControl::BrowserControl(QWebEngineView *view, QObject *parent)
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
#if SEB_HAS_QTWEBENGINE
    return view_ && view_->history()->canGoBack();
#else
    return false;
#endif
}

bool BrowserControl::canNavigateForwards() const
{
#if SEB_HAS_QTWEBENGINE
    return view_ && view_->history()->canGoForward();
#else
    return false;
#endif
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
#if SEB_HAS_QTWEBENGINE
    if (view_) {
        view_->back();
    }
#endif
}

void BrowserControl::navigateForwards()
{
#if SEB_HAS_QTWEBENGINE
    if (view_) {
        view_->forward();
    }
#endif
}

void BrowserControl::reload()
{
#if SEB_HAS_QTWEBENGINE
    if (view_) {
        view_->reload();
    }
#endif
}

}  // namespace seb::browser
