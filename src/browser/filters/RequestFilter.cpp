#include "RequestFilter.h"

#include "RuleFactory.h"

namespace seb::browser::filters {
namespace {

bool matchesRule(const RuleVariant &variant, const QString &url)
{
    return std::visit([&url](const auto &compiledRule) {
        return compiledRule.matches(url);
    }, variant);
}

}  // namespace

RequestFilter::RequestFilter(const seb::FilterSettings &settings)
{
    for (const auto &rule : settings.rules) {
        CompiledRule compiled{RuleFactory::create(rule), rule.result};
        if (rule.result == seb::FilterResult::Allow) {
            allowRules_.push_back(std::move(compiled));
        } else {
            blockRules_.push_back(std::move(compiled));
        }
    }
}

bool RequestFilter::isAllowed(const QString &url) const
{
    for (const auto &rule : blockRules_) {
        if (matchesRule(rule.rule, url)) {
            return false;
        }
    }

    for (const auto &rule : allowRules_) {
        if (matchesRule(rule.rule, url)) {
            return true;
        }
    }

    return defaultResult_ == seb::FilterResult::Allow;
}

}  // namespace seb::browser::filters
