#include "request_filter.h"

#include <QRegularExpression>
#include <QUrl>

namespace seb::browser {
namespace {

QString wildcardToRegexPattern(const QString &expression)
{
    QString pattern = QRegularExpression::escape(expression);
    pattern.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
    pattern.replace(QStringLiteral("\\?"), QStringLiteral("."));
    return QStringLiteral("^%1$").arg(pattern);
}

}  // namespace

RequestFilter::RequestFilter(const seb::FilterSettings &settings)
    : settings_(settings)
{
    rules_.reserve(settings.rules.size());
    for (const seb::FilterRuleSettings &rule : settings.rules) {
        const QRegularExpression expression = compileRule(rule);
        if (expression.isValid()) {
            rules_.push_back({expression, rule.result});
        }
    }
}

FilterDecision RequestFilter::evaluate(const QUrl &url, QWebEngineUrlRequestInfo::ResourceType resourceType) const
{
    const bool mainRequest = isMainRequest(resourceType);
    if ((mainRequest && !settings_.processMainRequests) || (!mainRequest && !settings_.processContentRequests)) {
        return FilterDecision::NoMatch;
    }

    const QString decoded = QUrl::fromPercentEncoding(url.toEncoded());
    for (const CompiledRule &rule : rules_) {
        if (rule.expression.match(decoded).hasMatch()) {
            return rule.result == seb::FilterResult::Allow ? FilterDecision::Allow : FilterDecision::Block;
        }
    }

    return FilterDecision::NoMatch;
}

QRegularExpression RequestFilter::compileRule(const seb::FilterRuleSettings &rule)
{
    switch (rule.type) {
    case seb::FilterRuleType::Regex:
        return QRegularExpression(rule.expression, QRegularExpression::CaseInsensitiveOption);
    case seb::FilterRuleType::Simplified:
        return QRegularExpression(wildcardToRegexPattern(rule.expression), QRegularExpression::CaseInsensitiveOption);
    }

    return {};
}

bool RequestFilter::isMainRequest(QWebEngineUrlRequestInfo::ResourceType resourceType)
{
    return resourceType == QWebEngineUrlRequestInfo::ResourceTypeMainFrame ||
           resourceType == QWebEngineUrlRequestInfo::ResourceTypeNavigationPreloadMainFrame;
}

}  // namespace seb::browser
