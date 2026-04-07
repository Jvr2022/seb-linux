#include "resource_loader.h"

#include "password_container.h"

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace seb::settingsinternal {
namespace {

SebSettings browserFallbackSettings(const QUrl &url)
{
    SebSettings settings = defaultSettings();
    settings.browser.startUrl = url.toString();
    settings.browser.homeUrl = settings.browser.startUrl;
    settings.browser.useStartUrlAsHomeUrl = true;
    settings.browser.mainWindow.showToolbar = true;
    settings.browser.mainWindow.showHomeButton = true;
    settings.browser.mainWindow.showReloadButton = true;
    return settings;
}

}  // namespace

ResourceLoadResult loadSettingsFromNetworkResource(
    const QUrl &url,
    const std::function<QString (bool)> &passwordProvider)
{
    ResourceLoadResult result;

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("SEB Linux Qt/0.1.0"));

    QEventLoop loop;
    QNetworkReply *reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    const QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    const int httpStatus = statusCode.isValid() ? statusCode.toInt() : 0;
    const QByteArray contentType = reply->header(QNetworkRequest::ContentTypeHeader).toByteArray();
    const QByteArray body = reply->readAll();

    const bool isSebExtension = url.path().endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive);

    if (reply->error() != QNetworkReply::NoError && httpStatus == 0) {
        result.error = QStringLiteral("Failed to download '%1': %2").arg(url.toString(), reply->errorString());
        reply->deleteLater();
        return result;
    }

    if (!isSebExtension && (httpStatus == 401 || contentType.startsWith("text/html") || looksLikeHtml(body))) {
        result.settings = browserFallbackSettings(url);
        result.browserUrl = url;
        result.ok = true;
        result.openInBrowser = true;
        result.warnings << QStringLiteral("The remote resource returned an authenticated or HTML response and was opened in the browser.");
        reply->deleteLater();
        return result;
    }

    const LoadResult loaded = loadSettingsFromData(body, url.toString(), passwordProvider);
    result.settings = loaded.settings;
    result.error = loaded.error;
    result.warnings = loaded.warnings;
    result.ok = loaded.ok;
    reply->deleteLater();
    return result;
}

}  // namespace seb::settingsinternal
