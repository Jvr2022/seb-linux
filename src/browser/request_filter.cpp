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

RequestFilter::RequestFilter(const seb::settings::browser::FilterSettings &settings)
    : settings_(settings)
{
    allowRules_.reserve(settings.Rules.size());
    blockRules_.reserve(settings.Rules.size());
    for (const auto &rule : settings.Rules) {
        const QRegularExpression expression = compileRule(rule);
        if (expression.isValid()) {
            if (rule.Result == seb::settings::browser::filter::FilterResult::Allow && rule.Active) {
                allowRules_.push_back({expression, rule.Result});
            } else if (rule.Active) {
                blockRules_.push_back({expression, rule.Result});
            }
        }
    }
}

QRegularExpression RequestFilter::compileRule(
    const seb::settings::browser::filter::FilterRuleSettings &rule)
{
    switch (rule.Type) {
    case seb::settings::browser::filter::FilterRuleType::Regex:
        return QRegularExpression(rule.Expression, QRegularExpression::CaseInsensitiveOption);
    case seb::settings::browser::filter::FilterRuleType::Simplified:
        return QRegularExpression(
            wildcardToRegexPattern(rule.Expression), QRegularExpression::CaseInsensitiveOption);
    }

    return {};
}

FilterDecision RequestFilter::evaluate(const QUrl &url, contracts::ResourceType resourceType) const
{
    const bool mainRequest = isMainRequest(resourceType);
    if ((mainRequest && !settings_.ProcessMainRequests) ||
        (!mainRequest && !settings_.ProcessContentRequests)) {
        return FilterDecision::NoMatch;
    }

    const QString decoded = QUrl::fromPercentEncoding(url.toEncoded());
    for (const CompiledRule &rule : blockRules_) {
        if (rule.expression.match(decoded).hasMatch()) {
            return FilterDecision::Block;
        }
    }

    for (const CompiledRule &rule : allowRules_) {
        if (rule.expression.match(decoded).hasMatch()) {
            return FilterDecision::Allow;
        }
    }

    return FilterDecision::Block;
}
bool RequestFilter::isMainRequest(contracts::ResourceType resourceType)
{
    return resourceType == contracts::ResourceType::MainFrame ||
           resourceType == contracts::ResourceType::NavigationPreloadMainFrame;
}

}  // namespace seb::browser
