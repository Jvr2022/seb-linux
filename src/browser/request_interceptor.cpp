#include "request_interceptor.h"

#include <QUrl>
#include <QWebEngineUrlRequestInfo>

namespace seb::browser {

RequestInterceptor::RequestInterceptor(const seb::SebSettings &settings, QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
    , settings_(settings)
    , filter_(settings.browser.filter)
    , keyGenerator_(settings)
{
}

void RequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    const QUrl requestUrl = info.requestUrl();
    if (!requestUrl.isValid()) {
        return;
    }

    if (requestUrl.scheme() == QStringLiteral("mailto")) {
        info.block(true);
        return;
    }

    if (requestUrl.scheme() == QStringLiteral("seb") || requestUrl.scheme() == QStringLiteral("sebs")) {
        info.redirect(replaceSebScheme(requestUrl));
        return;
    }

    const FilterDecision decision = filter_.evaluate(requestUrl, info.resourceType());
    if (decision == FilterDecision::Block) {
        info.block(true);
        return;
    }

    if (shouldAppendHeaders(info)) {
        if (settings_.browser.sendConfigurationKey && !settings_.browser.configurationKey.isEmpty()) {
            info.setHttpHeader("X-SafeExamBrowser-ConfigKeyHash", keyGenerator_.configurationKeyHash(requestUrl));
        }
        if (settings_.browser.sendBrowserExamKey) {
            info.setHttpHeader("X-SafeExamBrowser-RequestHash", keyGenerator_.requestHash(requestUrl));
        }
    }
}

bool RequestInterceptor::shouldAppendHeaders(const QWebEngineUrlRequestInfo &info) const
{
    if (info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeMainFrame ||
        info.resourceType() == QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadMainFrame) {
        return true;
    }

    return sameHost(info.firstPartyUrl(), info.requestUrl());
}

bool RequestInterceptor::sameHost(const QUrl &lhs, const QUrl &rhs)
{
    return lhs.isValid() && rhs.isValid() && lhs.host().compare(rhs.host(), Qt::CaseInsensitive) == 0;
}

QUrl RequestInterceptor::replaceSebScheme(const QUrl &url)
{
    QUrl replaced(url);
    if (url.scheme() == QStringLiteral("seb")) {
        replaced.setScheme(QStringLiteral("http"));
    } else if (url.scheme() == QStringLiteral("sebs")) {
        replaced.setScheme(QStringLiteral("https"));
    }
    return replaced;
}

}  // namespace seb::browser
