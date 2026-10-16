#include "simplified_rule.h"

namespace seb::browser::filters::rules {
namespace {

const QRegularExpression &urlDelimiterPattern()
{
    static const QRegularExpression pattern(
        QStringLiteral(
            R"(^(?:([^:\/\?#]*)\:\/\/)?(?:([^\@]*?)(?:\:([^\@]*))?\@)?(?:([^\/\:\?#]*))?(?:\:([0-9\*]*))?([^\?#]*)?(?:\?([^#]*))?(?:#(.*))?$)"));
    return pattern;
}

bool hasAllowedCharacter(const QString &expression)
{
    static const QRegularExpression allowed(QStringLiteral(R"([a-zA-Z0-9\*]+)"));
    return allowed.match(expression).hasMatch();
}

}  // namespace

SimplifiedRule::SimplifiedRule(const QString &expression)
{
    if (!hasAllowedCharacter(expression)) {
        return;
    }

    const QRegularExpressionMatch match = urlDelimiterPattern().match(expression);
    if (!match.hasMatch()) {
        return;
    }

    const QString scheme = match.captured(1);
    if (!scheme.isEmpty()) {
        scheme_ = build(replaceWildcard(QRegularExpression::escape(scheme)));
        if (!scheme_.isValid()) {
            return;
        }
    }

    const QString username = match.captured(2);
    if (!username.isEmpty()) {
        QString userInfo = QRegularExpression::escape(username);
        const QString password = match.captured(3);
        if (password.isEmpty()) {
            userInfo += QStringLiteral("(:.*)?");
        } else {
            userInfo += QStringLiteral(":") + QRegularExpression::escape(password);
        }
        userInfo_ = build(replaceWildcard(userInfo));
        if (!userInfo_.isValid()) {
            return;
        }
    }

    QString host = match.captured(4);
    const bool alphanumeric =
        QRegularExpression(QStringLiteral(R"(^[a-zA-Z0-9]+$)")).match(host).hasMatch();
    const bool exactSubdomain = host.startsWith(QLatin1Char('.'));
    if (exactSubdomain) {
        host.remove(0, 1);
    }
    host = replaceWildcard(QRegularExpression::escape(host));
    if (!alphanumeric && !exactSubdomain) {
        host = QStringLiteral("(.+?\\.)*") + host;
    }
    host_ = build(host);
    if (!host_.isValid()) {
        return;
    }

    const QString port = match.captured(5);
    bool portOk = false;
    const int parsedPort = port.toInt(&portOk);
    if (portOk) {
        port_ = parsedPort;
    }

    QString path = match.captured(6);
    if (!path.trimmed().isEmpty() && path != QStringLiteral("/")) {
        path = replaceWildcard(QRegularExpression::escape(path));
        path += path.endsWith(QLatin1Char('/'))
            ? QStringLiteral("?")
            : QStringLiteral("/?");
        path_ = build(path);
        if (!path_.isValid()) {
            return;
        }
    }

    const QString query = match.captured(7);
    if (!query.trimmed().isEmpty()) {
        if (query == QStringLiteral(".")) {
            query_ = build(QStringLiteral("\\??"));
        } else {
            query_ = build(
                QStringLiteral("\\??") +
                replaceWildcard(QRegularExpression::escape(query)));
        }
        if (!query_.isValid()) {
            return;
        }
    }

    const QString fragment = match.captured(8);
    if (!fragment.trimmed().isEmpty()) {
        fragment_ = build(
            QStringLiteral("#?") +
            replaceWildcard(QRegularExpression::escape(fragment)));
        if (!fragment_.isValid()) {
            return;
        }
    }

    valid_ = true;
}

bool SimplifiedRule::isValid() const
{
    return valid_;
}

bool SimplifiedRule::isMatch(const QUrl &url) const
{
    if (!valid_ || !url.isValid()) {
        return false;
    }

    const QString scheme = url.scheme();
    const QString userInfo = url.userInfo();
    const QString host = url.host(QUrl::FullyDecoded);
    const QString path = url.path(QUrl::FullyDecoded);
    QString query = url.query(QUrl::FullyDecoded);
    if (!query.isEmpty()) {
        query.prepend(QLatin1Char('?'));
    }
    QString fragment = url.fragment(QUrl::FullyDecoded);
    if (!fragment.isEmpty()) {
        fragment.prepend(QLatin1Char('#'));
    }

    bool match = true;
    match &= !scheme_.isValid() || scheme_.match(scheme).hasMatch();
    match &= !userInfo_.isValid() || userInfo_.match(userInfo).hasMatch();
    match &= host_.match(host).hasMatch();
    match &= !port_.has_value() || *port_ == effectivePort(url);
    match &= !path_.isValid() || path_.match(path).hasMatch();
    match &= !query_.isValid() || query_.match(query).hasMatch();
    match &= !fragment_.isValid() || fragment_.match(fragment).hasMatch();
    return match;
}

QRegularExpression SimplifiedRule::build(const QString &expression)
{
    return QRegularExpression(
        QStringLiteral("^(%1)$").arg(expression),
        QRegularExpression::CaseInsensitiveOption);
}

QString SimplifiedRule::replaceWildcard(QString expression)
{
    expression.replace(QStringLiteral("\\*"), QStringLiteral(".*"));
    return expression;
}

int SimplifiedRule::effectivePort(const QUrl &url)
{
    if (url.port() >= 0) {
        return url.port();
    }
    if (url.scheme().compare(QStringLiteral("http"), Qt::CaseInsensitive) == 0) {
        return 80;
    }
    if (url.scheme().compare(QStringLiteral("https"), Qt::CaseInsensitive) == 0) {
        return 443;
    }
    if (url.scheme().compare(QStringLiteral("ftp"), Qt::CaseInsensitive) == 0) {
        return 21;
    }
    return -1;
}

}  // namespace seb::browser::filters::rules
