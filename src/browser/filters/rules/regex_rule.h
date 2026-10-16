#pragma once

#include "i_rule.h"

#include <QRegularExpression>

namespace seb::browser::filters::rules {

class RegexRule final : public IRule
{
public:
    explicit RegexRule(const QString &expression);

    bool isValid() const;
    bool isMatch(const QUrl &url) const override;

private:
    QRegularExpression expression_;
};

}  // namespace seb::browser::filters::rules
