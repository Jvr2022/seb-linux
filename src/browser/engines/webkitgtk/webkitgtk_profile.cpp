#include "webkitgtk_profile.h"

#if !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK

#ifdef signals
#pragma push_macro("signals")
#undef signals
#define SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#endif
#include <webkit2/webkit2.h>
#ifdef SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#pragma pop_macro("signals")
#undef SEB_WEBKITGTK_RESTORE_SIGNALS_MACRO
#endif

#include <QByteArray>
#include <QVector>

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
    if (d->context) {
        webkit_web_context_set_spell_checking_enabled(d->context, enabled);
    }
}

void WebKitGtkProfile::setSpellCheckLanguages(const QStringList &languages)
{
    if (!d->context) {
        return;
    }

    QVector<QByteArray> encodedLanguages;
    encodedLanguages.reserve(languages.size());

    QVector<const gchar *> spellCheckingLanguages;
    spellCheckingLanguages.reserve(languages.size() + 1);

    for (const QString &language : languages) {
        const QString trimmedLanguage = language.trimmed();
        if (trimmedLanguage.isEmpty()) {
            continue;
        }

        encodedLanguages.push_back(trimmedLanguage.toUtf8());
        spellCheckingLanguages.push_back(encodedLanguages.constLast().constData());
    }

    spellCheckingLanguages.push_back(nullptr);

    webkit_web_context_set_spell_checking_languages(
        d->context,
        spellCheckingLanguages.size() > 1 ? spellCheckingLanguages.constData() : nullptr);
}

void WebKitGtkProfile::setHttpUserAgent(const QString &userAgent)
{
    // Agent is typically set on WebKitSettings per WebView, but can be managed here
}

void WebKitGtkProfile::setUrlRequestInterceptor(seb::browser::contracts::IRequestInterceptor *interceptor)
{
    // Use WebKitUserContentManager or custom URI scheme handler
}

void WebKitGtkProfile::deleteAllCookies()
{
    WebKitCookieManager *cookieManager = webkit_web_context_get_cookie_manager(d->context);
    webkit_cookie_manager_delete_all_cookies(cookieManager);
}

}  // namespace seb::browser

#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
