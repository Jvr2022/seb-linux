#include "request_interceptor.h"

#include <QUrl>

namespace seb::browser {

RequestInterceptor::RequestInterceptor(
    const seb::settings::browser::BrowserSettings &settings,
    seb::configuration::contracts::cryptography::IKeyGenerator &keyGenerator)
    : settings_(settings)
    , filter_(settings.Filter)
    , keyGenerator_(keyGenerator)
{
}

void RequestInterceptor::interceptRequest(contracts::IRequest &request)
{
    const QUrl requestUrl = request.requestUrl();
    if (!requestUrl.isValid()) {
        return;
    }

    if (requestUrl.scheme() == QStringLiteral("mailto")) {
        request.block(true);
        return;
    }

    if (requestUrl.scheme() == QStringLiteral("seb") || requestUrl.scheme() == QStringLiteral("sebs")) {
        request.redirect(replaceSebScheme(requestUrl));
        return;
    }

    const FilterDecision decision = filter_.evaluate(requestUrl, request.resourceType());
    if (decision == FilterDecision::Block) {
        request.block(true);
        return;
    }

    if (shouldAppendHeaders(request)) {
        const QString encodedUrl = QString::fromLatin1(requestUrl.toEncoded(QUrl::FullyEncoded));
        if (settings_.SendConfigurationKey) {
            request.setHttpHeader(
                "X-SafeExamBrowser-ConfigKeyHash",
                keyGenerator_.calculateConfigurationKeyHash(settings_.ConfigurationKey, encodedUrl).toLatin1());
        }
        if (settings_.SendBrowserExamKey) {
            request.setHttpHeader(
                "X-SafeExamBrowser-RequestHash",
                keyGenerator_.calculateBrowserExamKeyHash(
                    settings_.ConfigurationKey,
                    settings_.BrowserExamKeySalt,
                    encodedUrl)
                    .toLatin1());
        }
    }
}

bool RequestInterceptor::shouldAppendHeaders(const contracts::IRequest &request) const
{
    const auto type = request.resourceType();
    if (type == contracts::ResourceType::MainFrame ||
        type == contracts::ResourceType::NavigationPreloadMainFrame) {
        return true;
    }

    return sameHost(request.firstPartyUrl(), request.requestUrl());
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
