#pragma once

#if !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK

#include "../../contracts/i_webview.h"

namespace seb::browser {

class WebKitGtkProfile;

class WebKitGtkWebView : public contracts::IWebView
{
    Q_OBJECT

public:
    explicit WebKitGtkWebView(WebKitGtkProfile *profile, QWidget *parent = nullptr);
    ~WebKitGtkWebView() override;

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

    void setNavigationRequestDelegate(IWebView::NavigationRequestDelegate delegate) override;

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace seb::browser

#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
