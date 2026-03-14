#pragma once

#include <QRegularExpression>
#include <QString>

namespace seb::browser::filters::rules {

class RegexRule
{
public:
    explicit RegexRule(const QString &pattern);

    bool isValid() const;
    bool matches(const QString &url) const;

private:
    QRegularExpression regex_;
};

}  // namespace seb::browser::filters::rules
