#include "webkitgtk_view.h"
#include "webkitgtk_profile.h"
#include "browser/contracts/i_request_interceptor.h"

#if !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK

#ifdef signals
#pragma push_macro("signals")
#undef signals
#define SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#endif
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#ifdef SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#pragma pop_macro("signals")
#undef SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#endif

#include <QVBoxLayout>
#include <QByteArray>
#include <QDir>
#include <QStandardPaths>
#include <QStringList>
#include <QWidget>
#include <QWindow>

namespace seb::browser {

class WebKitGtkWebView::Private
{
public:
    QWidget *containerWidget = nullptr;
    GtkWidget *gtkWindow = nullptr;
    GtkWidget *webView = nullptr;
    WebKitGtkProfile *profile = nullptr;
    NavigationRequestDelegate navigationDelegate;
    FileSelectionDelegate fileSelectionDelegate;
};

class WebKitNavigationRequest final : public contracts::IRequest
{
public:
    WebKitNavigationRequest(
        WebKitURIRequest *request,
        const QUrl &firstPartyUrl,
        WebKitPolicyDecision *decision)
        : request_(request)
        , firstPartyUrl_(firstPartyUrl)
        , decision_(decision)
    {
    }

    QUrl requestUrl() const override
    {
        const gchar *uri = request_ ? webkit_uri_request_get_uri(request_) : nullptr;
        return uri ? QUrl(QString::fromUtf8(uri)) : QUrl();
    }

    QUrl firstPartyUrl() const override { return firstPartyUrl_; }
    contracts::ResourceType resourceType() const override { return contracts::ResourceType::MainFrame; }

    void block(bool shouldBlock) override
    {
        blocked_ = shouldBlock;
        if (blocked_ && decision_) {
            webkit_policy_decision_ignore(decision_);
        }
    }

    void redirect(const QUrl &url) override
    {
        if (request_) {
            webkit_uri_request_set_uri(
                request_,
                url.toString(QUrl::FullyEncoded).toUtf8().constData());
        }
    }

    void setHttpHeader(const QByteArray &name, const QByteArray &value) override
    {
        if (request_) {
            SoupMessageHeaders *headers = webkit_uri_request_get_http_headers(request_);
            if (headers) {
                soup_message_headers_replace(headers, name.constData(), value.constData());
            }
        }
    }

    bool blocked() const { return blocked_; }

private:
    WebKitURIRequest *request_ = nullptr;
    QUrl firstPartyUrl_;
    WebKitPolicyDecision *decision_ = nullptr;
    bool blocked_ = false;
};

WebKitGtkWebView::WebKitGtkWebView(WebKitGtkProfile *profile, QWidget *parent)
    : IWebView(parent)
    , d(std::make_unique<Private>())
{
    d->profile = profile;
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
    d->webView = profile && profile->nativeContext()
        ? webkit_web_view_new_with_context(profile->nativeContext())
        : webkit_web_view_new();

    if (profile) {
        WebKitSettings *settings = webkit_settings_new();
        const QByteArray userAgent = profile->httpUserAgent().toUtf8();
        if (!userAgent.isEmpty()) {
            webkit_settings_set_user_agent(settings, userAgent.constData());
        }
        webkit_web_view_set_settings(WEBKIT_WEB_VIEW(d->webView), settings);
        g_object_unref(settings);

        g_signal_connect(d->webView, "resource-load-started", G_CALLBACK(+[](WebKitWebView *,
                                                                      WebKitWebResource *,
                                                                      WebKitURIRequest *request,
                                                                      gpointer userData) {
            auto *requestProfile = static_cast<WebKitGtkProfile *>(userData);
            const QByteArray language = requestProfile->httpAcceptLanguage().toUtf8();
            if (!language.isEmpty() && request) {
                SoupMessageHeaders *headers = webkit_uri_request_get_http_headers(request);
                if (!headers) {
                    return;
                }
                soup_message_headers_replace(
                    headers,
                    "Accept-Language",
                    language.constData());
            }
        }), profile);
    }
    
    // Connect WebKitGTK signals (load-changed, decide-policy, etc.)
    g_signal_connect(d->webView, "load-changed", G_CALLBACK(+[](WebKitWebView *, WebKitLoadEvent event, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView*>(data);
        if (event == WEBKIT_LOAD_COMMITTED) {
            emit self->urlChanged(self->url());
        }
    }), this);

    g_signal_connect(d->webView, "decide-policy", G_CALLBACK(+[](WebKitWebView *,
                                                                    WebKitPolicyDecision *decision,
                                                                    WebKitPolicyDecisionType type,
                                                                    gpointer data) -> gboolean {
        auto *self = static_cast<WebKitGtkWebView *>(data);
        if (type != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
            return FALSE;
        }

        auto *navigation = WEBKIT_NAVIGATION_POLICY_DECISION(decision);
        WebKitNavigationAction *action =
            webkit_navigation_policy_decision_get_navigation_action(navigation);
        WebKitURIRequest *request = action
            ? webkit_navigation_action_get_request(action)
            : nullptr;
        const gchar *uri = request ? webkit_uri_request_get_uri(request) : nullptr;
        const gchar *frameName = action
            ? webkit_navigation_action_get_frame_name(action)
            : nullptr;
        const bool isMainFrame = !frameName || *frameName == '\0';
        QUrl url = uri ? QUrl(QString::fromUtf8(uri)) : QUrl();

        if (isMainFrame && self->d->profile) {
            if (auto *interceptor = self->d->profile->urlRequestInterceptor()) {
                WebKitNavigationRequest intercepted(request, self->url(), decision);
                interceptor->interceptRequest(intercepted);
                if (intercepted.blocked()) {
                    return TRUE;
                }
                url = intercepted.requestUrl();
            }
        }

        const bool allowed = !self->d->navigationDelegate ||
            self->d->navigationDelegate(url, isMainFrame);

        if (allowed) {
            webkit_policy_decision_use(decision);
        } else {
            webkit_policy_decision_ignore(decision);
        }
        return TRUE;
    }), this);

    g_signal_connect(d->webView, "notify::title", G_CALLBACK(+[](WebKitWebView *view, GParamSpec *, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView*>(data);
        const char *title = webkit_web_view_get_title(view);
        if (title) {
            emit self->titleChanged(QString::fromUtf8(title));
        }
    }), this);

    g_signal_connect(d->webView, "drag-drop", G_CALLBACK(+[](GtkWidget *,
                                                               GdkDragContext *context,
                                                               gint,
                                                               gint,
                                                               guint time,
                                                               gpointer) -> gboolean {
        gtk_drag_finish(context, FALSE, FALSE, time);
        return TRUE;
    }), this);

    g_signal_connect(d->webView, "run-file-chooser", G_CALLBACK(+[](WebKitWebView *,
                                                                       WebKitFileChooserRequest *request,
                                                                       gpointer data) -> gboolean {
        auto *self = static_cast<WebKitGtkWebView *>(data);
        if (!self->d->fileSelectionDelegate) {
            webkit_file_chooser_request_cancel(request);
            return TRUE;
        }

        QStringList oldFiles;
        if (const gchar * const *selected =
                webkit_file_chooser_request_get_selected_files(request)) {
            for (int i = 0; selected[i] != nullptr; ++i) {
                oldFiles.append(QString::fromUtf8(selected[i]));
            }
        }

        QStringList acceptedMimeTypes;
        if (const gchar * const *mimeTypes =
                webkit_file_chooser_request_get_mime_types(request)) {
            for (int i = 0; mimeTypes[i] != nullptr; ++i) {
                acceptedMimeTypes.append(QString::fromUtf8(mimeTypes[i]));
            }
        }

        // Match QWebEnginePage's chooser modes used by BrowserWindow:
        // 0 = one file, 1 = multiple files.
        const int mode = webkit_file_chooser_request_get_select_multiple(request) ? 1 : 0;
        const QStringList selectedFiles = self->d->fileSelectionDelegate(
            mode, oldFiles, acceptedMimeTypes);
        if (selectedFiles.isEmpty()) {
            webkit_file_chooser_request_cancel(request);
            return TRUE;
        }

        QVector<QByteArray> encodedFiles;
        QVector<const gchar *> filePointers;
        encodedFiles.reserve(selectedFiles.size());
        filePointers.reserve(selectedFiles.size() + 1);
        for (const QString &file : selectedFiles) {
            encodedFiles.append(file.toUtf8());
            filePointers.append(encodedFiles.constLast().constData());
        }
        filePointers.append(nullptr);
        webkit_file_chooser_request_select_files(request, filePointers.constData());
        return TRUE;
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

void WebKitGtkWebView::download(const QUrl &url, const QString &suggestedFileName)
{
    Q_UNUSED(suggestedFileName);
    if (d->webView) {
        webkit_web_view_download_uri(
            WEBKIT_WEB_VIEW(d->webView),
            url.toString(QUrl::FullyEncoded).toUtf8().constData());
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

void WebKitGtkWebView::setFileSelectionDelegate(FileSelectionDelegate delegate)
{
    d->fileSelectionDelegate = std::move(delegate);
}

}  // namespace seb::browser
#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
