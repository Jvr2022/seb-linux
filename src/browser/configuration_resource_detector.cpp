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
    QUrl converted(url);
    if (url.scheme().compare(QStringLiteral("seb"), Qt::CaseInsensitive) == 0) {
        converted.setScheme(QStringLiteral("http"));
    } else if (url.scheme().compare(QStringLiteral("sebs"), Qt::CaseInsensitive) == 0) {
        converted.setScheme(QStringLiteral("https"));
    }
    return converted;
}

}  // namespace seb::browser
