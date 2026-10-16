#pragma once

#include <QUrl>

namespace seb::browser::filters::rules {

class IRule
{
public:
    virtual ~IRule() = default;
    virtual bool isMatch(const QUrl &url) const = 0;
};

}  // namespace seb::browser::filters::rules
