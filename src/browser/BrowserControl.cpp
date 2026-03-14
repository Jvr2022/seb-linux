#include "BrowserControl.h"

#include <QWebEngineHistory>
#include <QWebEngineView>

namespace seb::browser {

BrowserControl::BrowserControl(QWebEngineView *view, QObject *parent)
    : QObject(parent)
    , view_(view)
{
}

QString BrowserControl::address() const
{
    return view_ ? view_->url().toString() : QString();
}

bool BrowserControl::canNavigateBackwards() const
{
    return view_ && view_->history()->canGoBack();
}

bool BrowserControl::canNavigateForwards() const
{
    return view_ && view_->history()->canGoForward();
}

void BrowserControl::navigateTo(const QString &address)
{
    if (view_) view_->setUrl(QUrl::fromUserInput(address));
}

void BrowserControl::navigateBackwards()
{
    if (view_) view_->back();
}

void BrowserControl::navigateForwards()
{
    if (view_) view_->forward();
}

void BrowserControl::reload()
{
    if (view_) view_->reload();
}

}  // namespace seb::browser
