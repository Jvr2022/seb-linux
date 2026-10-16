#pragma once

#include "i_rule.h"

#include <QRegularExpression>

#include <optional>

namespace seb::browser::filters::rules {

class SimplifiedRule final : public IRule
{
public:
    explicit SimplifiedRule(const QString &expression);

    bool isValid() const;
    bool isMatch(const QUrl &url) const override;

private:
    static QRegularExpression build(const QString &expression);
    static QString replaceWildcard(QString expression);
    static int effectivePort(const QUrl &url);

    bool valid_ = false;
    QRegularExpression scheme_;
    QRegularExpression userInfo_;
    QRegularExpression host_;
    std::optional<int> port_;
    QRegularExpression path_;
    QRegularExpression query_;
    QRegularExpression fragment_;
};

}  // namespace seb::browser::filters::rules
