#pragma once

#include <optional>
#include <QRegularExpression>
#include <QString>

namespace seb::browser::filters::rules {

class SimplifiedRule
{
public:
    explicit SimplifiedRule(const QString &pattern);

    bool isValid() const;
    bool matches(const QString &url) const;

private:
    static QString replaceWildcard(QString expression);
    static QRegularExpression buildExpression(const QString &expression);

    void parseExpression(const QString &expression);
    void parseScheme(QString expression);
    void parseUserInfo(QString userName, QString password);
    void parseHost(QString expression);
    void parsePort(const QString &expression);
    void parsePath(QString expression);
    void parseQuery(QString expression);
    void parseFragment(QString expression);

    bool valid_ = false;
    QRegularExpression fragment_;
    QRegularExpression host_;
    QRegularExpression path_;
    std::optional<int> port_;
    QRegularExpression query_;
    QRegularExpression scheme_;
    QRegularExpression userInfo_;
};

}  // namespace seb::browser::filters::rules
