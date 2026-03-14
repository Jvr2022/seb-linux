#include "RegexRule.h"

namespace seb::browser::filters::rules {

RegexRule::RegexRule(const QString &pattern)
    : regex_(pattern, QRegularExpression::CaseInsensitiveOption)
{
}

bool RegexRule::isValid() const
{
    return regex_.isValid();
}

bool RegexRule::matches(const QString &url) const
{
    return regex_.isValid() && regex_.match(url).hasMatch();
}

}  // namespace seb::browser::filters::rules
