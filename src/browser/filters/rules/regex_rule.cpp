#include "regex_rule.h"

namespace seb::browser::filters::rules {

RegexRule::RegexRule(const QString &expression)
    : expression_(expression, QRegularExpression::CaseInsensitiveOption)
{
}

bool RegexRule::isValid() const
{
    return expression_.isValid();
}

bool RegexRule::isMatch(const QUrl &url) const
{
    return expression_.match(url.toString(QUrl::FullyDecoded)).hasMatch();
}

}  // namespace seb::browser::filters::rules
