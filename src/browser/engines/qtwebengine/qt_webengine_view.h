#pragma once

#if SEB_HAS_QTWEBENGINE

#include "browser/contracts/i_webview.h"
#include "settings/AppSettings.h"
#include "settings/Browser/WindowSettings.h"
#include <memory>

QT_BEGIN_NAMESPACE
class QWebEngineView;
class QWebChannel;
class QPrinter;
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
    void configureBrowser(const seb::settings::AppSettings &settings,
                          const seb::settings::browser::WindowSettings &window) override;
    void initializeClipboard(seb::browser::Clipboard *clipboard) override;
    void setZoomFactor(double factor) override;
    void print() override;

    QUrl url() const override;
    void setUrl(const QUrl &url) override;
    void download(const QUrl &url, const QString &suggestedFileName = {}) override;

    void back() override;
    void forward() override;
    void reload() override;
    void openDevTools() override;
    bool canGoBack() const override;
    bool canGoForward() const override;

    void findText(const QString &text) override;

    void setNavigationRequestDelegate(contracts::IWebView::NavigationRequestDelegate delegate) override;
    void setFileSelectionDelegate(contracts::IWebView::FileSelectionDelegate delegate) override;
    void initializeSebApi(
        seb::configuration::contracts::cryptography::IKeyGenerator *keyGenerator,
        const seb::settings::browser::BrowserSettings &settings,
        const seb::configuration::contracts::AppConfig &appConfig) override;
    const contracts::IWebView::NavigationRequestDelegate& delegate() const { return navigationDelegate_; }
    const contracts::IWebView::FileSelectionDelegate& fileSelectionDelegate() const { return fileSelectionDelegate_; }

private:
    bool eventFilter(QObject *watched, QEvent *event) override;

    QWebEngineView *view_ = nullptr;
    QWebChannel *webChannel_ = nullptr;
    bool sebApiInitialized_ = false;
    bool configured_ = false;
    bool printing_ = false;
    seb::settings::security::ClipboardPolicy clipboardPolicy_ = seb::settings::security::ClipboardPolicy::Isolated;
    seb::settings::monitoring::KeyboardSettings keyboardSettings_;
    seb::settings::monitoring::MouseSettings mouseSettings_;
    seb::browser::Clipboard *clipboard_ = nullptr;
    std::function<void(const QString &)> clipboardCommand_;
    seb::settings::browser::BrowserSettings settings_;
    seb::settings::browser::WindowSettings windowSettings_;
    std::unique_ptr<QPrinter> printer_;
    contracts::IWebView::NavigationRequestDelegate navigationDelegate_;
    contracts::IWebView::FileSelectionDelegate fileSelectionDelegate_;
};

}  // namespace seb::browser

#endif
