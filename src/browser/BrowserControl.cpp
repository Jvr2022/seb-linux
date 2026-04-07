#include "BrowserControl.h"

#include "engine/browser_view.h"

#include <QUrl>

namespace seb::browser {

BrowserControl::BrowserControl(seb::browser::engine::BrowserView *view, QObject *parent)
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
    return view_ && view_->canGoBack();
}

bool BrowserControl::canNavigateForwards() const
{
    return view_ && view_->canGoForward();
}

void BrowserControl::navigateTo(const QString &address)
{
    if (view_) view_->load(QUrl::fromUserInput(address));
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
