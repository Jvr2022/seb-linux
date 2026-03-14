#pragma once

#include "../../../seb_settings.h"
#include "IRule.h"
#include "request.h"

namespace seb::browser::contracts::filters {

class IRequestFilter
{
public:
    virtual ~IRequestFilter() = default;

    virtual seb::FilterResult defaultResult() const = 0;
    virtual void setDefaultResult(seb::FilterResult result) = 0;
    virtual void load(IRule *rule) = 0;
    virtual seb::FilterResult process(const Request &request) const = 0;
};

}  // namespace seb::browser::contracts::filters
