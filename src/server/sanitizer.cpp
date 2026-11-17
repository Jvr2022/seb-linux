#include "sanitizer.h"

namespace seb::server {

QUrl Sanitizer::baseAddress(const QString &serverUrl)
{
    QUrl url(serverUrl.trimmed(), QUrl::StrictMode);
    if (!url.isValid() || (url.scheme() != QStringLiteral("http") &&
                           url.scheme() != QStringLiteral("https")) ||
        url.host().isEmpty() || !url.userInfo().isEmpty() || url.hasFragment() || url.hasQuery()) {
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
    if (!base.isValid() || endpoint.trimmed().isEmpty()) return {};
    if (endpoint.trimmed().startsWith(QStringLiteral("//"))) return {};
    const QUrl relative(apiEndpoint(endpoint), QUrl::StrictMode);
    const QUrl resolved = base.resolved(relative);
    if (!relative.isValid() || !resolved.isValid() || !resolved.userInfo().isEmpty() || resolved.hasFragment() ||
        resolved.scheme() != base.scheme() || resolved.host().compare(base.host(), Qt::CaseInsensitive) != 0 ||
        resolved.port(resolved.scheme() == QStringLiteral("https") ? 443 : 80) !=
            base.port(base.scheme() == QStringLiteral("https") ? 443 : 80)) return {};
    return resolved;
}

} // namespace seb::server
