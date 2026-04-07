#include "webkitgtk_profile.h"

#if !SEB_HAS_QTWEBENGINE

#include <webkit2/webkit2.h>

namespace seb::browser {

class WebKitGtkProfile::Private
{
public:
    WebKitWebContext *context = nullptr;
};

WebKitGtkProfile::WebKitGtkProfile(QObject *parent)
    : IWebProfile(parent)
    , d(std::make_unique<Private>())
{
    // Create an ephemeral or persistent WebKitContext
    d->context = webkit_web_context_new();
}

WebKitGtkProfile::~WebKitGtkProfile()
{
    if (d->context) {
        g_object_unref(d->context);
    }
}

void WebKitGtkProfile::setPersistentStoragePath(const QString &path)
{
    // WebKitGTK data dir config
}

void WebKitGtkProfile::setCachePath(const QString &path)
{
    // WebKitGTK cache dir config
}

void WebKitGtkProfile::setDownloadPath(const QString &path)
{
    // Handled at download request level in WebKitGTK
}

void WebKitGtkProfile::setPdfViewerEnabled(bool enabled)
{
    // Not directly supported out of the box in WebKitGTK without Evince plugin
}

void WebKitGtkProfile::setSpellCheckEnabled(bool enabled)
{
    WebKitWebContext *context = webkit_web_context_get_default();
    webkit_web_context_set_spell_checking_enabled(context, enabled);
}

void WebKitGtkProfile::setSpellCheckLanguages(const QStringList &languages)
{
    WebKitWebContext *context = webkit_web_context_get_default();
    QString langStr = languages.join(QStringLiteral(","));
    webkit_web_context_set_spell_checking_languages(context, langStr.toUtf8().constData());
}

void WebKitGtkProfile::setHttpUserAgent(const QString &userAgent)
{
    // Agent is typically set on WebKitSettings per WebView, but can be managed here
}

void WebKitGtkProfile::setUrlRequestInterceptor(seb::browser::RequestInterceptor *interceptor)
{
    // Use WebKitUserContentManager or custom URI scheme handler
}

void WebKitGtkProfile::deleteAllCookies()
{
    WebKitCookieManager *cookieManager = webkit_web_context_get_cookie_manager(d->context);
    webkit_cookie_manager_delete_all_cookies(cookieManager);
}

}  // namespace seb::browser

#endif
