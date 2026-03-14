#pragma once

#include "../../../seb_settings.h"
#include "request.h"

namespace seb::browser::contracts::filters {

class IRule
{
public:
    virtual ~IRule() = default;

    virtual seb::FilterResult result() const = 0;
    virtual void initialize(const seb::FilterRuleSettings &settings) = 0;
    virtual bool isMatch(const Request &request) const = 0;
};

}  // namespace seb::browser::contracts::filters
