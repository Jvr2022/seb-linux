#include "sanitizer.h"

namespace seb::server {

QUrl Sanitizer::baseAddress(const QString &serverUrl)
{
    QUrl url(serverUrl.trimmed());
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") &&
                           url.scheme() != QStringLiteral("https")) ||
        url.host().isEmpty()) {
        return {};
    }

    QString path = url.path();
    if (!path.endsWith(QLatin1Char('/'))) {
        path += QLatin1Char('/');
        url.setPath(path);
    }
    return url;
}

QString Sanitizer::apiEndpoint(const QString &endpoint)
{
    const QString value = endpoint.trimmed();
    const QUrl absolute(value);
    if (absolute.isValid() && !absolute.scheme().isEmpty()) {
        return value;
    }
    return value.startsWith(QLatin1Char('/')) ? value.mid(1) : value;
}

QUrl Sanitizer::resolve(const QUrl &base, const QString &endpoint)
{
    const QString sanitized = apiEndpoint(endpoint);
    const QUrl absolute(sanitized);
    if (absolute.isValid() && (absolute.scheme() == QStringLiteral("http") ||
                               absolute.scheme() == QStringLiteral("https"))) {
        return absolute;
    }
    if (sanitized.isEmpty()) {
        return {};
    }
    return base.resolved(QUrl(sanitized));
}

}  // namespace seb::server
