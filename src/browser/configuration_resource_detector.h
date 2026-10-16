#pragma once

#include <QUrl>

namespace seb::browser {

class ConfigurationResourceDetector
{
public:
    static bool isConfigurationResource(
        const QUrl &url,
        const QString &suggestedFileName = {},
        const QString &mimeType = {});

    static QUrl downloadUrl(const QUrl &url);
};

}  // namespace seb::browser
