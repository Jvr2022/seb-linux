#include "configuration_resource_detector.h"

namespace seb::browser {

bool ConfigurationResourceDetector::isConfigurationResource(
    const QUrl &url,
    const QString &suggestedFileName,
    const QString &mimeType)
{
    const bool fileNameMatch = suggestedFileName.endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive);
    const bool pathMatch = url.path(QUrl::FullyDecoded).endsWith(QStringLiteral(".seb"), Qt::CaseInsensitive);
    const bool mimeMatch = mimeType.trimmed().compare(QStringLiteral("application/seb"), Qt::CaseInsensitive) == 0;
    const bool dataMatch = url.toString(QUrl::FullyDecoded).contains(
        QStringLiteral("application/seb"), Qt::CaseInsensitive);

    return fileNameMatch || pathMatch || mimeMatch || dataMatch;
}

QUrl ConfigurationResourceDetector::downloadUrl(const QUrl &url)
{
    const QString encoded = url.toString(QUrl::FullyEncoded);
    const bool isDataConfiguration = encoded.contains(
        QStringLiteral("application/seb"),
        Qt::CaseInsensitive);
    const QString scheme = url.scheme();

    if (isDataConfiguration &&
        (scheme.compare(QStringLiteral("seb"), Qt::CaseInsensitive) == 0 ||
         scheme.compare(QStringLiteral("sebs"), Qt::CaseInsensitive) == 0)) {
        const qsizetype separator = encoded.indexOf(QStringLiteral("://"));
        if (separator >= 0) {
            return QUrl(
                QStringLiteral("data:") +
                encoded.mid(separator + 3));
        }
    }

    QUrl converted(url);
    if (scheme.compare(QStringLiteral("seb"), Qt::CaseInsensitive) == 0) {
        converted.setScheme(QStringLiteral("http"));
    } else if (scheme.compare(QStringLiteral("sebs"), Qt::CaseInsensitive) == 0) {
        converted.setScheme(QStringLiteral("https"));
    }
    return converted;
}

}  // namespace seb::browser
