#include "qt_webengine_view.h"

#if SEB_HAS_QTWEBENGINE
#include "qt_webengine_profile.h"
#include <QWebEngineView>
#include <QWebEngineHistory>
#include <QWebEngineNewWindowRequest>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

namespace seb::browser {

class InterceptingPage : public QWebEnginePage
{
public:
    InterceptingPage(QWebEngineProfile *profile, QtWebEngineWebView *wrapper, QObject *parent = nullptr)
        : QWebEnginePage(profile, parent), wrapper_(wrapper) {}

protected:
    bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame) override
    {
        if (wrapper_->delegate() && !wrapper_->delegate()(url, isMainFrame)) {
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
    
private:
    QtWebEngineWebView *wrapper_;
};

QtWebEngineWebView::QtWebEngineWebView(QtWebEngineProfile *profile, QWidget *parent)
    : IWebView(parent)
{
    view_ = new QWebEngineView(parent);
    auto *page = new InterceptingPage(profile->nativeProfile(), this, view_);
    view_->setPage(page);

    if (profile->devBypass()) {
        QWebEngineScript script;
        script.setName(QStringLiteral("seb-stealth"));
        script.setSourceCode(QStringLiteral(
            "(function() {"
            "  const mask = (obj, prop, val) => {"
            "    try { Object.defineProperty(obj, prop, { get: () => val, enumerable: true, configurable: true }); } catch (e) {}"
            "  };"
            "  mask(navigator, 'webdriver', false);"
            "  mask(navigator, 'plugins', [1, 2, 3, 4, 5]);"
            "  mask(navigator, 'languages', ['en-US', 'en']);"
            "  window.chrome = { runtime: {} };"
            "  mask(document, 'hidden', false);"
            "  mask(document, 'visibilityState', 'visible');"
            "  window.addEventListener('blur', e => e.stopImmediatePropagation(), true);"
            "  document.addEventListener('blur', e => e.stopImmediatePropagation(), true);"
            "  window.addEventListener('mouseleave', e => e.stopImmediatePropagation(), true);"
            "})();"
        ));
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setRunsOnSubFrames(true);
        profile->nativeProfile()->scripts()->insert(script);
    }

    connect(view_, &QWebEngineView::urlChanged, this, &QtWebEngineWebView::urlChanged);
    connect(view_, &QWebEngineView::titleChanged, this, &QtWebEngineWebView::titleChanged);
    
    connect(page, &QWebEnginePage::newWindowRequested, this, [this](auto &request) {
        bool handled = false;
        emit this->newWindowRequested(request.requestedUrl(), handled);
        if (handled) {
            // In our simplified API we handle the window opening externally, so we drop the request object.
        }
    });

    connect(page, &QWebEnginePage::proxyAuthenticationRequired,
            this, &QtWebEngineWebView::proxyAuthenticationRequired);
}

QtWebEngineWebView::~QtWebEngineWebView()
{
    // delete view_; // Parent ownership will handle this usually, but view_ is a QWidget.
}

QWidget *QtWebEngineWebView::widget() const
{
    return view_;
}

QUrl QtWebEngineWebView::url() const
{
    return view_->url();
}

void QtWebEngineWebView::setUrl(const QUrl &url)
{
    view_->setUrl(url);
}

void QtWebEngineWebView::back()
{
    view_->back();
}

void QtWebEngineWebView::forward()
{
    view_->forward();
}

void QtWebEngineWebView::reload()
{
    view_->reload();
}

void QtWebEngineWebView::openDevTools()
{
    auto *devTools = new QWebEngineView();
    auto *devPage = new QWebEnginePage(view_->page()->profile(), devTools);
    view_->page()->setDevToolsPage(devPage);
    devTools->setPage(devPage);
    devTools->setAttribute(Qt::WA_DeleteOnClose);
    devTools->resize(1100, 700);
    devTools->show();
}

bool QtWebEngineWebView::canGoBack() const
{
    return view_->history()->canGoBack();
}

bool QtWebEngineWebView::canGoForward() const
{
    return view_->history()->canGoForward();
}

void QtWebEngineWebView::findText(const QString &text)
{
    view_->page()->findText(QString());
    view_->page()->findText(text);
}

void QtWebEngineWebView::setNavigationRequestDelegate(contracts::IWebView::NavigationRequestDelegate delegate)
{
    navigationDelegate_ = std::move(delegate);
}

}  // namespace seb::browser
#endif
