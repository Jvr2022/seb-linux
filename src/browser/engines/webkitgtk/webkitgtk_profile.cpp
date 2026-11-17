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
#include <QSet>

namespace seb::browser {

class WebKitGtkProfile::Private
{
public:
    WebKitWebContext *context = nullptr;
    QString defaultUserAgent;
    QString userAgent;
    QString acceptLanguage;
    QSet<WebKitDownload *> downloads;
    std::shared_ptr<contracts::IRequestInterceptor> urlRequestInterceptor;
};

struct DownloadContext
{
    QPointer<WebKitGtkProfile> owner;
    QUuid id;
    QUrl url;
    bool failed = false;
};

WebKitGtkProfile::WebKitGtkProfile(bool ephemeral, QObject *parent)
    : IWebProfile(parent)
    , d(std::make_unique<Private>())
{
    d->context = ephemeral ? webkit_web_context_new_ephemeral() : webkit_web_context_new();

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

        profile->d->downloads.insert(download);
        g_object_ref(download);
        auto *downloadContext = new DownloadContext;
        downloadContext->owner = profile;
        downloadContext->id = request.id;
        downloadContext->url = request.url;
        g_signal_connect(download, "failed", G_CALLBACK(+[](WebKitDownload *, GError *, gpointer data) {
            static_cast<DownloadContext *>(data)->failed = true;
        }), downloadContext);
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
                result.success = !context->failed && !result.localPath.isEmpty();
                context->owner->d->downloads.remove(finished);
                emit context->owner->downloadFinished(result);
            }
            g_signal_handlers_disconnect_by_data(finished, context);
            delete context;
            g_object_unref(finished);
        }), downloadContext);
    }), this);
}

WebKitGtkProfile::~WebKitGtkProfile()
{
    cancelDownloads();
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

void WebKitGtkProfile::setUrlRequestInterceptor(std::shared_ptr<seb::browser::contracts::IRequestInterceptor> interceptor)
{
    d->urlRequestInterceptor = std::move(interceptor);
}

seb::browser::contracts::IRequestInterceptor *WebKitGtkProfile::urlRequestInterceptor() const
{
    return d->urlRequestInterceptor.get();
}

void WebKitGtkProfile::deleteAllCookies()
{
    WebKitCookieManager *cookieManager = webkit_web_context_get_cookie_manager(d->context);
    webkit_cookie_manager_delete_all_cookies(cookieManager);
}

void WebKitGtkProfile::clearHttpCache(std::function<void(bool)> done)
{
    auto *manager = d->context ? webkit_web_context_get_website_data_manager(d->context) : nullptr;
    if (!manager) { if (done) done(false); return; }
    struct Completion { QPointer<WebKitGtkProfile> owner; std::function<void(bool)> done; };
    auto *completion = new Completion{this, std::move(done)};
    webkit_website_data_manager_clear(manager,
        static_cast<WebKitWebsiteDataTypes>(WEBKIT_WEBSITE_DATA_MEMORY_CACHE | WEBKIT_WEBSITE_DATA_DISK_CACHE),
        0, nullptr, +[](GObject *source, GAsyncResult *result, gpointer data) {
            std::unique_ptr<Completion> callback(static_cast<Completion *>(data));
            GError *error = nullptr;
            const bool success = webkit_website_data_manager_clear_finish(WEBKIT_WEBSITE_DATA_MANAGER(source), result, &error);
            if (error) g_error_free(error);
            if (callback->owner && callback->done) callback->done(success);
        }, completion);
}

void WebKitGtkProfile::cancelDownloads()
{
    const auto downloads = d->downloads;
    for (auto *download : downloads) webkit_download_cancel(download);
}

}  // namespace seb::browser

#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
