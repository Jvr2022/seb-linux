#pragma once

#include "../../../seb_settings.h"
#include "IRule.h"

namespace seb::browser::contracts::filters {

class IRuleFactory
{
public:
    virtual ~IRuleFactory() = default;
    virtual IRule *createRule(seb::FilterRuleType type) = 0;
};

}  // namespace seb::browser::contracts::filters
