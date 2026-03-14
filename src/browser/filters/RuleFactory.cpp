#include "RuleFactory.h"

namespace seb::browser::filters {

RuleVariant RuleFactory::create(const seb::FilterRuleSettings &rule)
{
    if (rule.type == seb::FilterRuleType::Regex) {
        return rules::RegexRule(rule.expression);
    }
    return rules::SimplifiedRule(rule.expression);
}

}  // namespace seb::browser::filters
