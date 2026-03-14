#pragma once

#include "../../seb_settings.h"

#include "rules/RegexRule.h"
#include "rules/SimplifiedRule.h"

#include <variant>

namespace seb::browser::filters {

using RuleVariant = std::variant<rules::RegexRule, rules::SimplifiedRule>;

class RuleFactory
{
public:
    static RuleVariant create(const seb::FilterRuleSettings &rule);
};

}  // namespace seb::browser::filters
