#include "webkitgtk_view.h"

#if !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

namespace seb::browser {

class WebKitGtkWebView::Private
{
public:
    QWidget *containerWidget = nullptr;
    GtkWidget *gtkWindow = nullptr;
    GtkWidget *webView = nullptr;
    NavigationRequestDelegate navigationDelegate;
};

WebKitGtkWebView::WebKitGtkWebView(WebKitGtkProfile *profile, QWidget *parent)
    : IWebView(parent)
    , d(std::make_unique<Private>())
{
    d->containerWidget = new QWidget(parent);
    
    // NOTE: Full embedding of GTK into Qt6 requires bridging the X11 Window ID.
    // This is a minimal stub to ensure compilation and linkage to GTK+3 and WebKit2.
    // In a production environment, you would use gtk_plug_new() and QWindow::fromWinId().
    
    // gtk_init must be called exactly once per process.
    static bool gtkInitialized = false;
    if (!gtkInitialized) {
        gtk_init(nullptr, nullptr);
        gtkInitialized = true;
    }
    d->webView = webkit_web_view_new();
    
    // Connect WebKitGTK signals (load-changed, decide-policy, etc.)
    g_signal_connect(d->webView, "load-changed", G_CALLBACK(+[](WebKitWebView *, WebKitLoadEvent event, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView*>(data);
        if (event == WEBKIT_LOAD_COMMITTED) {
            emit self->urlChanged(self->url());
        }
    }), this);

    g_signal_connect(d->webView, "notify::title", G_CALLBACK(+[](WebKitWebView *view, GParamSpec *, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView*>(data);
        const char *title = webkit_web_view_get_title(view);
        if (title) {
            emit self->titleChanged(QString::fromUtf8(title));
        }
    }), this);
}

WebKitGtkWebView::~WebKitGtkWebView()
{
    if (d->webView) {
        gtk_widget_destroy(d->webView);
    }
}

QWidget *WebKitGtkWebView::widget() const
{
    return d->containerWidget;
}

QUrl WebKitGtkWebView::url() const
{
    if (!d->webView) return {};
    const char *uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(d->webView));
    return uri ? QUrl(QString::fromUtf8(uri)) : QUrl();
}

void WebKitGtkWebView::setUrl(const QUrl &url)
{
    if (d->webView) {
        webkit_web_view_load_uri(WEBKIT_WEB_VIEW(d->webView), url.toString().toUtf8().constData());
    }
}

void WebKitGtkWebView::back()
{
    if (d->webView) {
        webkit_web_view_go_back(WEBKIT_WEB_VIEW(d->webView));
    }
}

void WebKitGtkWebView::forward()
{
    if (d->webView) {
        webkit_web_view_go_forward(WEBKIT_WEB_VIEW(d->webView));
    }
}

void WebKitGtkWebView::reload()
{
    if (d->webView) {
        webkit_web_view_reload(WEBKIT_WEB_VIEW(d->webView));
    }
}

void WebKitGtkWebView::openDevTools()
{
    if (d->webView) {
        WebKitWebInspector *inspector = webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(d->webView));
        if (inspector) {
            webkit_web_inspector_show(inspector);
        }
    }
}

bool WebKitGtkWebView::canGoBack() const
{
    return d->webView && webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(d->webView));
}

bool WebKitGtkWebView::canGoForward() const
{
    return d->webView && webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(d->webView));
}

void WebKitGtkWebView::findText(const QString &text)
{
    if (!d->webView) return;
    WebKitFindController *finder = webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(d->webView));
    webkit_find_controller_search(finder, text.toUtf8().constData(), WEBKIT_FIND_OPTIONS_NONE, G_MAXUINT);
}

void WebKitGtkWebView::setNavigationRequestDelegate(NavigationRequestDelegate delegate)
{
    d->navigationDelegate = std::move(delegate);
}

}  // namespace seb::browser
#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
