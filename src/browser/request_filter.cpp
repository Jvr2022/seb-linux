#include "request_filter.h"

#include "filters/rules/regex_rule.h"
#include "filters/rules/simplified_rule.h"

#include <QDebug>

namespace seb::browser {
RequestFilter::RequestFilter(const seb::settings::browser::FilterSettings &settings)
    : settings_(settings)
{
    allowRules_.reserve(settings.Rules.size());
    blockRules_.reserve(settings.Rules.size());
    for (const auto &rule : settings.Rules) {
        if (!rule.Active) {
            continue;
        }

        std::unique_ptr<filters::rules::IRule> compiled;
        if (rule.Type == seb::settings::browser::filter::FilterRuleType::Regex) {
            auto candidate = std::make_unique<filters::rules::RegexRule>(rule.Expression);
            if (!candidate->isValid()) {
                qWarning() << "Ignoring invalid SEB regular-expression filter rule:" << rule.Expression;
                continue;
            }
            compiled = std::move(candidate);
        } else {
            auto candidate = std::make_unique<filters::rules::SimplifiedRule>(rule.Expression);
            if (!candidate->isValid()) {
                qWarning() << "Ignoring invalid SEB simplified filter rule:" << rule.Expression;
                continue;
            }
            compiled = std::move(candidate);
        }

        LoadedRule loaded{std::move(compiled), rule.Result};
        if (rule.Result == seb::settings::browser::filter::FilterResult::Allow) {
            allowRules_.push_back(std::move(loaded));
        } else {
            blockRules_.push_back(std::move(loaded));
        }
    }
}

FilterDecision RequestFilter::evaluate(const QUrl &url, contracts::ResourceType resourceType) const
{
    const bool mainRequest = isMainRequest(resourceType);
    if ((mainRequest && !settings_.ProcessMainRequests) ||
        (!mainRequest && !settings_.ProcessContentRequests)) {
        return FilterDecision::NoMatch;
    }

    if (mainRequest &&
        url.scheme().compare(QStringLiteral("chrome-extension"), Qt::CaseInsensitive) == 0) {
        return FilterDecision::NoMatch;
    }

    for (const LoadedRule &rule : blockRules_) {
        if (rule.rule->isMatch(url)) {
            return FilterDecision::Block;
        }
    }

    for (const LoadedRule &rule : allowRules_) {
        if (rule.rule->isMatch(url)) {
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
