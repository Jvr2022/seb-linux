#include "qtwebengine_browser_view.h"

#include "../../seb_session.h"

#include <QWebEngineHistory>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineProfile>
#include <QWebEngineView>

namespace seb::browser::engine {

QtWebEngineBrowserPage::QtWebEngineBrowserPage(SebSession &session, QObject *parent)
    : QWebEnginePage(session.profile(), parent)
    , session_(session)
{
}

void QtWebEngineBrowserPage::setNavigationPolicy(BrowserView::NavigationPolicy policy)
{
    navigationPolicy_ = std::move(policy);
}

void QtWebEngineBrowserPage::setUploadsAllowed(bool allowed)
{
    uploadsAllowed_ = allowed;
}

bool QtWebEngineBrowserPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (navigationPolicy_) {
        if (!navigationPolicy_(url, isMainFrame)) {
            return false;
        }
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

QStringList QtWebEngineBrowserPage::chooseFiles(
    FileSelectionMode mode,
    const QStringList &oldFiles,
    const QStringList &acceptedMimeTypes)
{
    if (!uploadsAllowed_) {
        return {};
    }
    return QWebEnginePage::chooseFiles(mode, oldFiles, acceptedMimeTypes);
}

QtWebEngineBrowserView::QtWebEngineBrowserView(
    SebSession &session,
    QWebEngineProfile *profile,
    QWidget *parentWidget,
    QObject *parent)
    : BrowserView(parent)
{
    view_ = new QWebEngineView(parentWidget);
    page_ = new QtWebEngineBrowserPage(session, view_);
    if (profile) {
        page_->setProfile(profile);
    }
    view_->setPage(page_);

    connect(view_, &QWebEngineView::urlChanged, this, &BrowserView::urlChanged);
    connect(view_, &QWebEngineView::titleChanged, this, &BrowserView::titleChanged);

    connect(page_, &QWebEnginePage::newWindowRequested, this, [this](QWebEngineNewWindowRequest &request) {
        const QUrl target = request.requestedUrl();
        const QUrl opener = view_ ? view_->url() : QUrl();
        emit newWindowRequested(target, opener);
        // The caller will decide whether to open in same window or create a new one.
        // We intentionally do not call request.openIn() here.
    });
}

QtWebEngineBrowserView::~QtWebEngineBrowserView() = default;

QWidget *QtWebEngineBrowserView::widget()
{
    return view_;
}

QUrl QtWebEngineBrowserView::url() const
{
    return view_ ? view_->url() : QUrl();
}

QString QtWebEngineBrowserView::title() const
{
    return view_ ? view_->title() : QString();
}

bool QtWebEngineBrowserView::canGoBack() const
{
    return view_ && view_->history() && view_->history()->canGoBack();
}

bool QtWebEngineBrowserView::canGoForward() const
{
    return view_ && view_->history() && view_->history()->canGoForward();
}

void QtWebEngineBrowserView::load(const QUrl &url)
{
    if (view_) {
        view_->setUrl(url);
    }
}

void QtWebEngineBrowserView::back()
{
    if (view_) {
        view_->back();
    }
}

void QtWebEngineBrowserView::forward()
{
    if (view_) {
        view_->forward();
    }
}

void QtWebEngineBrowserView::reload()
{
    if (view_) {
        view_->reload();
    }
}

void QtWebEngineBrowserView::setNavigationPolicy(NavigationPolicy policy)
{
    if (page_) {
        page_->setNavigationPolicy(std::move(policy));
    }
}

void QtWebEngineBrowserView::setUploadsAllowed(bool allowed)
{
    if (page_) {
        page_->setUploadsAllowed(allowed);
    }
}

void QtWebEngineBrowserView::findText(const QString &text)
{
    if (page_) {
        page_->findText(text);
    }
}

void QtWebEngineBrowserView::openDevTools()
{
    if (!page_) {
        return;
    }

    auto *devTools = new QWebEngineView();
    auto *devPage = new QWebEnginePage(page_->profile(), devTools);
    page_->setDevToolsPage(devPage);
    devTools->setPage(devPage);
    devTools->setAttribute(Qt::WA_DeleteOnClose);
    devTools->resize(1100, 700);
    devTools->show();
}

QWebEnginePage *QtWebEngineBrowserView::page() const
{
    return page_;
}

}  // namespace seb::browser::engine

