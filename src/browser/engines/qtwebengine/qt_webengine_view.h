#pragma once

#if SEB_HAS_QTWEBENGINE

#include "browser/contracts/i_webview.h"

QT_BEGIN_NAMESPACE
class QWebEngineView;
QT_END_NAMESPACE

namespace seb::browser {

class QtWebEngineProfile;

class QtWebEngineWebView : public contracts::IWebView
{
    Q_OBJECT

public:
    explicit QtWebEngineWebView(QtWebEngineProfile *profile, QWidget *parent = nullptr);
    ~QtWebEngineWebView() override;

    QWidget *widget() const override;

    QUrl url() const override;
    void setUrl(const QUrl &url) override;

    void back() override;
    void forward() override;
    void reload() override;
    void openDevTools() override;
    bool canGoBack() const override;
    bool canGoForward() const override;

    void findText(const QString &text) override;

    void setNavigationRequestDelegate(contracts::IWebView::NavigationRequestDelegate delegate) override;
    const contracts::IWebView::NavigationRequestDelegate& delegate() const { return navigationDelegate_; }

private:
    QWebEngineView *view_ = nullptr;
    contracts::IWebView::NavigationRequestDelegate navigationDelegate_;
};

}  // namespace seb::browser

#endif
