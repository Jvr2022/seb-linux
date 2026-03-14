#include "SimplifiedRule.h"

#include <QRegularExpressionMatch>
#include <QUrl>

namespace seb::browser::filters::rules {
namespace {

constexpr auto kUrlDelimiterPattern =
    R"((?:([^\:]*)\://)?(?:([^\:\@]*)(?:\:([^\@]*))?\@)?(?:([^/\:\?#]*))?(?:\:([0-9\*]*))?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?)";

bool hasContent(const QString &expression)
{
    static const QRegularExpression pattern(QStringLiteral(R"([a-zA-Z0-9\*]+)"));
    return pattern.match(expression).hasMatch();
}

}  // namespace

SimplifiedRule::SimplifiedRule(const QString &pattern)
{
    if (!hasContent(pattern)) {
        return;
    }

    const QRegularExpression delimiter(QString::fromLatin1(kUrlDelimiterPattern));
    const auto delimiterMatch = delimiter.match(pattern);
    if (!delimiterMatch.hasMatch()) {
        return;
    }

    parseExpression(pattern);
    valid_ = host_.isValid();
}

bool SimplifiedRule::isValid() const
{
    return valid_;
}

bool SimplifiedRule::matches(const QString &url) const
{
    if (!valid_) {
        return false;
    }

    const QUrl parsed(url, QUrl::StrictMode);
    if (!parsed.isValid()) {
        return false;
    }

    bool isMatch = true;
    isMatch = isMatch && (!scheme_.isValid() || scheme_.match(parsed.scheme()).hasMatch());
    isMatch = isMatch && (!userInfo_.isValid() || userInfo_.match(parsed.userInfo()).hasMatch());
    isMatch = isMatch && host_.match(parsed.host()).hasMatch();
    isMatch = isMatch && (!port_.has_value() || port_.value() == parsed.port());
    isMatch = isMatch && (!path_.isValid() || path_.match(parsed.path()).hasMatch());
    isMatch = isMatch && (!query_.isValid() || query_.match(QStringLiteral("?") + parsed.query()).hasMatch() ||
                          query_.match(parsed.query()).hasMatch());
    isMatch = isMatch && (!fragment_.isValid() || fragment_.match(QStringLiteral("#") + parsed.fragment()).hasMatch() ||
                          fragment_.match(parsed.fragment()).hasMatch());
    return isMatch;
}

QString SimplifiedRule::replaceWildcard(QString expression)
{
    expression.replace(QStringLiteral(R"(\*)"), QStringLiteral(".*"));
    return expression;
}

QRegularExpression SimplifiedRule::buildExpression(const QString &expression)
{
    return QRegularExpression(QStringLiteral("^%1$").arg(expression), QRegularExpression::CaseInsensitiveOption);
}

void SimplifiedRule::parseExpression(const QString &expression)
{
    const QRegularExpression delimiter(QString::fromLatin1(kUrlDelimiterPattern));
    const auto match = delimiter.match(expression);

    parseScheme(match.captured(1));
    parseUserInfo(match.captured(2), match.captured(3));
    parseHost(match.captured(4));
    parsePort(match.captured(5));
    parsePath(match.captured(6));
    parseQuery(match.captured(7));
    parseFragment(match.captured(8));
}

void SimplifiedRule::parseScheme(QString expression)
{
    if (expression.isEmpty()) {
        return;
    }

    expression = replaceWildcard(QRegularExpression::escape(expression));
    scheme_ = buildExpression(expression);
}

void SimplifiedRule::parseUserInfo(QString userName, QString password)
{
    if (userName.isEmpty()) {
        return;
    }

    userName = QRegularExpression::escape(userName);
    password = QRegularExpression::escape(password);

    QString expression = password.isEmpty()
        ? QStringLiteral("%1(:.*)?").arg(userName)
        : QStringLiteral("%1:%2").arg(userName, password);
    expression = replaceWildcard(expression);

    userInfo_ = buildExpression(expression);
}

void SimplifiedRule::parseHost(QString expression)
{
    static const QRegularExpression alphaNumeric(QStringLiteral(R"(^[a-zA-Z0-9]+$)"));
    const bool isAlphaNumeric = alphaNumeric.match(expression).hasMatch();
    const bool matchExactSubdomain = expression.startsWith(QLatin1Char('.'));

    if (matchExactSubdomain) {
        expression.remove(0, 1);
    }

    expression = replaceWildcard(QRegularExpression::escape(expression));
    if (!isAlphaNumeric && !matchExactSubdomain) {
        expression = QStringLiteral("(.+?\\.)*%1").arg(expression);
    }

    host_ = buildExpression(expression);
}

void SimplifiedRule::parsePort(const QString &expression)
{
    bool ok = false;
    const int parsed = expression.toInt(&ok);
    if (ok) {
        port_ = parsed;
    }
}

void SimplifiedRule::parsePath(QString expression)
{
    if (expression.trimmed().isEmpty() || expression == QStringLiteral("/")) {
        return;
    }

    expression = replaceWildcard(QRegularExpression::escape(expression));
    expression = expression.endsWith(QLatin1Char('/'))
        ? QStringLiteral("%1?").arg(expression)
        : QStringLiteral("%1/?").arg(expression);

    path_ = buildExpression(expression);
}

void SimplifiedRule::parseQuery(QString expression)
{
    if (expression.trimmed().isEmpty()) {
        return;
    }

    if (expression == QStringLiteral(".")) {
        query_ = buildExpression(QStringLiteral(R"(\??)"));
        return;
    }

    expression = replaceWildcard(QRegularExpression::escape(expression));
    query_ = buildExpression(QStringLiteral(R"(\??%1)").arg(expression));
}

void SimplifiedRule::parseFragment(QString expression)
{
    if (expression.trimmed().isEmpty()) {
        return;
    }

    expression = replaceWildcard(QRegularExpression::escape(expression));
    fragment_ = buildExpression(QStringLiteral("#?%1").arg(expression));
}

}  // namespace seb::browser::filters::rules
