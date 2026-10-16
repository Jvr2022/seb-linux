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
#include <QDir>
#include <QPointer>
#include <QVector>

namespace seb::browser {

class WebKitGtkProfile::Private
{
public:
    WebKitWebContext *context = nullptr;
    QString defaultUserAgent;
    QString userAgent;
    QString acceptLanguage;
    contracts::IRequestInterceptor *urlRequestInterceptor = nullptr;
};

struct DownloadContext
{
    QPointer<WebKitGtkProfile> owner;
    QUuid id;
    QUrl url;
};

WebKitGtkProfile::WebKitGtkProfile(QObject *parent)
    : IWebProfile(parent)
    , d(std::make_unique<Private>())
{
    // Create an ephemeral or persistent WebKitContext
    d->context = webkit_web_context_new();

    WebKitSettings *settings = webkit_settings_new();
    const gchar *userAgent = webkit_settings_get_user_agent(settings);
    d->defaultUserAgent = userAgent ? QString::fromUtf8(userAgent) : QString();
    g_object_unref(settings);

    g_signal_connect(d->context, "download-started", G_CALLBACK(+[](WebKitWebContext *, WebKitDownload *download, gpointer userData) {
        auto *profile = static_cast<WebKitGtkProfile *>(userData);
        WebKitURIRequest *uriRequest = webkit_download_get_request(download);
        WebKitURIResponse *response = webkit_download_get_response(download);

        contracts::DownloadRequest request;
        request.id = QUuid::createUuid();
        if (uriRequest) {
            const gchar *uri = webkit_uri_request_get_uri(uriRequest);
            if (uri) {
                request.url = QUrl(QString::fromUtf8(uri));
            }
        }
        if (response) {
            const gchar *mime = webkit_uri_response_get_mime_type(response);
            const gchar *fileName = webkit_uri_response_get_suggested_filename(response);
            if (mime) {
                request.mimeType = QString::fromUtf8(mime);
            }
            if (fileName) {
                request.suggestedFileName = QString::fromUtf8(fileName);
            }
        }

        emit profile->downloadRequested(request);
        if (!request.accepted) {
            webkit_download_cancel(download);
            return;
        }

        const QString fileName = request.targetFileName.isEmpty()
            ? request.suggestedFileName
            : request.targetFileName;
        const QString path = QDir(request.targetDirectory).filePath(fileName);
        webkit_download_set_destination(
            download,
            QUrl::fromLocalFile(path).toString(QUrl::FullyEncoded).toUtf8().constData());

        auto *downloadContext = new DownloadContext;
        downloadContext->owner = profile;
        downloadContext->id = request.id;
        downloadContext->url = request.url;
        g_signal_connect(download, "finished", G_CALLBACK(+[](WebKitDownload *finished, gpointer data) {
            auto *context = static_cast<DownloadContext *>(data);
            const gchar *destination = webkit_download_get_destination(finished);
            if (context->owner) {
                contracts::DownloadResult result;
                result.id = context->id;
                result.url = context->url;
                result.localPath = destination
                    ? QUrl(QString::fromUtf8(destination)).toLocalFile()
                    : QString();
                result.success = !result.localPath.isEmpty();
                emit context->owner->downloadFinished(result);
            }
            delete context;
        }), downloadContext);
    }), this);
}

WebKitGtkProfile::~WebKitGtkProfile()
{
    if (d->context) {
        g_object_unref(d->context);
    }
}

WebKitWebContext *WebKitGtkProfile::nativeContext() const
{
    return d->context;
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
    d->userAgent = userAgent;
}

QString WebKitGtkProfile::httpUserAgent() const
{
    if (!d->userAgent.isEmpty()) {
        return d->userAgent;
    }

    return d->defaultUserAgent;
}

QString WebKitGtkProfile::defaultHttpUserAgent() const
{
    return d->defaultUserAgent;
}

void WebKitGtkProfile::setHttpAcceptLanguage(const QString &language)
{
    d->acceptLanguage = language;
}

void WebKitGtkProfile::setPersistSessionCookies(bool persist)
{
    Q_UNUSED(persist);
}

QString WebKitGtkProfile::httpAcceptLanguage() const
{
    return d->acceptLanguage;
}

void WebKitGtkProfile::setUrlRequestInterceptor(seb::browser::contracts::IRequestInterceptor *interceptor)
{
    d->urlRequestInterceptor = interceptor;
}

seb::browser::contracts::IRequestInterceptor *WebKitGtkProfile::urlRequestInterceptor() const
{
    return d->urlRequestInterceptor;
}

void WebKitGtkProfile::deleteAllCookies()
{
    WebKitCookieManager *cookieManager = webkit_web_context_get_cookie_manager(d->context);
    webkit_cookie_manager_delete_all_cookies(cookieManager);
}

void WebKitGtkProfile::clearHttpCache()
{
    if (!d->context) {
        return;
    }

    WebKitWebsiteDataManager *manager = webkit_web_context_get_website_data_manager(d->context);
    if (manager) {
        webkit_website_data_manager_clear(
            manager,
            static_cast<WebKitWebsiteDataTypes>(
                WEBKIT_WEBSITE_DATA_MEMORY_CACHE | WEBKIT_WEBSITE_DATA_DISK_CACHE),
            0,
            nullptr,
            nullptr,
            nullptr);
    }
}

}  // namespace seb::browser

#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
