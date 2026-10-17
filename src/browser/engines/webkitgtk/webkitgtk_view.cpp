#include "webkitgtk_view.h"

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

#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

namespace seb::browser {
namespace {

// GTK/WebKit run on GLib's main context. This Qt build has no guarantee that
// QCoreApplication is dispatching GLib events for us (that only happens for
// free when Qt itself was built with the glib event-dispatcher backend), so
// we pump the pending GLib/GTK events on a timer from inside Qt's own event
// loop. Redundant iterations when nothing is pending are cheap no-ops, so
// this is safe to run continuously regardless of whether native glib
// integration is also active.
void ensureGtkPump() {
  static QTimer *pump = nullptr;
  if (pump) {
    return;
  }
  pump = new QTimer();
  QObject::connect(pump, &QTimer::timeout, []() {
    while (gtk_events_pending()) {
      gtk_main_iteration();
    }
  });
  pump->start(15);
}

} // namespace

class WebKitGtkWebView::Private {
public:
  QWidget *containerWidget = nullptr;
  GtkWidget *gtkWindow = nullptr;
  GtkWidget *webView = nullptr;
  NavigationRequestDelegate navigationDelegate;
};

WebKitGtkWebView::WebKitGtkWebView(WebKitGtkProfile *profile, QWidget *parent)
    : IWebView(parent), d(std::make_unique<Private>()) {
  d->containerWidget = new QWidget(parent);

  // NOTE: We deliberately do NOT try to embed the GTK/WebKit widget inside
  // the Qt widget tree (that would need XEmbed-style X11 window
  // reparenting, which is fragile and doesn't have a Wayland-native
  // equivalent). Instead we show WebKitGTK in its own real top-level
  // window and make it fullscreen, which is sufficient for a locked-down
  // kiosk browser: the student only ever needs to see the exam content
  // filling the screen, not a widget nested inside SEB's own chrome.
  // containerWidget stays an empty placeholder so BrowserWindow's layout
  // code (which expects view_->widget() to exist) keeps working.

  // 9/9/26: gtk_init must be called exactly once per process.
  static bool gtkInitialized = false;
  if (!gtkInitialized) {
    gtk_init(nullptr, nullptr);
    gtkInitialized = true;
  }

  ensureGtkPump();

  d->webView = webkit_web_view_new();

  d->gtkWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(d->gtkWindow), FALSE);
  gtk_container_add(GTK_CONTAINER(d->gtkWindow), d->webView);
  gtk_widget_show_all(d->gtkWindow);
  gtk_window_present(GTK_WINDOW(d->gtkWindow));
  gtk_window_fullscreen(GTK_WINDOW(d->gtkWindow));
  gtk_window_set_keep_above(GTK_WINDOW(d->gtkWindow), TRUE);

  // Connect WebKitGTK signals (load-changed, decide-policy... )
  g_signal_connect(
      d->webView, "load-changed",
      G_CALLBACK(+[](WebKitWebView *, WebKitLoadEvent event, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView *>(data);
        if (event == WEBKIT_LOAD_COMMITTED) {
          emit self->urlChanged(self->url());
        }
      }),
      this);

  g_signal_connect(
      d->webView, "notify::title",
      G_CALLBACK(+[](WebKitWebView *view, GParamSpec *, gpointer data) {
        auto *self = static_cast<WebKitGtkWebView *>(data);
        const char *title = webkit_web_view_get_title(view);
        if (title) {
          emit self->titleChanged(QString::fromUtf8(title));
        }
      }),
      this);
}

WebKitGtkWebView::~WebKitGtkWebView() {
  if (d->gtkWindow) {
    // Destroying the top-level window also destroys the webView widget
    // packed inside it.
    gtk_widget_destroy(d->gtkWindow);
  } else if (d->webView) {
    gtk_widget_destroy(d->webView);
  }
}

QWidget *WebKitGtkWebView::widget() const { return d->containerWidget; }

QUrl WebKitGtkWebView::url() const {
  if (!d->webView)
    return {};
  const char *uri = webkit_web_view_get_uri(WEBKIT_WEB_VIEW(d->webView));
  return uri ? QUrl(QString::fromUtf8(uri)) : QUrl();
}

void WebKitGtkWebView::setUrl(const QUrl &url) {
  if (d->webView) {
    webkit_web_view_load_uri(WEBKIT_WEB_VIEW(d->webView),
                             url.toString().toUtf8().constData());
  }
}

void WebKitGtkWebView::back() {
  if (d->webView) {
    webkit_web_view_go_back(WEBKIT_WEB_VIEW(d->webView));
  }
}

void WebKitGtkWebView::forward() {
  if (d->webView) {
    webkit_web_view_go_forward(WEBKIT_WEB_VIEW(d->webView));
  }
}

void WebKitGtkWebView::reload() {
  if (d->webView) {
    webkit_web_view_reload(WEBKIT_WEB_VIEW(d->webView));
  }
}

void WebKitGtkWebView::openDevTools() {
  if (d->webView) {
    WebKitWebInspector *inspector =
        webkit_web_view_get_inspector(WEBKIT_WEB_VIEW(d->webView));
    if (inspector) {
      webkit_web_inspector_show(inspector);
    }
  }
}

bool WebKitGtkWebView::canGoBack() const {
  return d->webView && webkit_web_view_can_go_back(WEBKIT_WEB_VIEW(d->webView));
}

bool WebKitGtkWebView::canGoForward() const {
  return d->webView &&
         webkit_web_view_can_go_forward(WEBKIT_WEB_VIEW(d->webView));
}

void WebKitGtkWebView::findText(const QString &text) {
  if (!d->webView)
    return;
  WebKitFindController *finder =
      webkit_web_view_get_find_controller(WEBKIT_WEB_VIEW(d->webView));
  webkit_find_controller_search(finder, text.toUtf8().constData(),
                                WEBKIT_FIND_OPTIONS_NONE, G_MAXUINT);
}

void WebKitGtkWebView::setNavigationRequestDelegate(
    NavigationRequestDelegate delegate) {
  d->navigationDelegate = std::move(delegate);
}

} // namespace seb::browser
#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
