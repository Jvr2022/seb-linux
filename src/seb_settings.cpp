#include "seb_settings.h"

#include "settings/password_container.h"
#include "settings/resource_loader.h"
#include "settings/settings_defaults.h"
#include "settings/settings_mapping.h"
#include "settings/settings_parsing.h"

#include <QFile>
#include <QUrl>

namespace seb {
namespace {

QUrl normalizeSebResourceUrl(const QString &resource)
{
    QUrl url = QUrl::fromUserInput(resource.trimmed());
    if (url.scheme() == QStringLiteral("seb")) {
        url.setScheme(QStringLiteral("http"));
    } else if (url.scheme() == QStringLiteral("sebs")) {
        url.setScheme(QStringLiteral("https"));
    }
    return url;
}

}  // namespace

SebSettings defaultSettings()
{
    return settingsinternal::createDefaultSettings();
}

LoadResult loadSettingsFromData(
    const QByteArray &sourceData,
    const QString &sourceName,
    const std::function<QString (bool)> &passwordProvider)
{
    LoadResult result;
    result.settings = defaultSettings();
    result.settings.sourceFile = sourceName;

    QByteArray raw = sourceData;
    if (raw.size() >= 2 &&
        static_cast<unsigned char>(raw.at(0)) == 0x1f &&
        static_cast<unsigned char>(raw.at(1)) == 0x8b) {
        QString error;
        raw = settingsinternal::inflateGzip(raw, &error);
        if (raw.isEmpty()) {
            result.error = error;
            return result;
        }
        result.warnings << QStringLiteral("Loaded gzip-compressed configuration data.");
    }

    if (!settingsinternal::startsWithXml(raw) && !raw.trimmed().startsWith('{')) {
        QString unwrapError;
        raw = settingsinternal::unwrapSebContainer(raw, &unwrapError, &result.warnings, passwordProvider);
        if (raw.isEmpty() && !unwrapError.isEmpty()) {
            result.error = unwrapError;
            return result;
        }
    }

    const QByteArray trimmed = raw.trimmed();
    QVariantMap data;
    QString error;

    if (trimmed.startsWith('{')) {
        data = settingsinternal::parseJson(trimmed, &error);
    } else if (trimmed.startsWith("<?xml") || trimmed.startsWith("<plist")) {
        data = settingsinternal::parsePlist(trimmed, &error);
    } else {
        result.error = QStringLiteral(
            "Unsupported configuration format. This Linux port currently supports JSON and unencrypted XML plist .seb files.");
        return result;
    }

    if (!error.isEmpty()) {
        result.error = error;
        return result;
    }

    if (settingsinternal::hasStructuredSections(data)) {
        settingsinternal::applyStructuredSettings(data, result.settings);
    } else {
        settingsinternal::applyRawSettings(data, result.settings);
    }

    if (result.settings.browser.configurationKey.isEmpty()) {
        result.settings.browser.configurationKey = settingsinternal::calculateConfigurationKey(data);
    }

    settingsinternal::finalizeSettings(result.settings);
    result.ok = true;
    return result;
}

LoadResult loadSettingsFromFile(const QString &path)
{
    QFile file(path);
    LoadResult result;
    result.settings = defaultSettings();
    result.settings.sourceFile = path;

    if (!file.open(QIODevice::ReadOnly)) {
        result.error = QStringLiteral("Failed to open '%1': %2").arg(path, file.errorString());
        return result;
    }

    return loadSettingsFromData(file.readAll(), path);
}

ResourceLoadResult loadSettingsFromResource(
    const QString &resource,
    const std::function<QString (bool)> &passwordProvider)
{
    ResourceLoadResult result;
    const QString trimmed = resource.trimmed();

    if (trimmed.isEmpty()) {
        result.error = QStringLiteral("No configuration resource was specified.");
        return result;
    }

    const QUrl url = normalizeSebResourceUrl(trimmed);
    if (!url.isValid() || url.isLocalFile() || url.scheme().isEmpty() || QFile::exists(trimmed)) {
        const QString path = url.isLocalFile() ? url.toLocalFile() : trimmed;
        QFile localFile(path);
        if (!localFile.open(QIODevice::ReadOnly)) {
            result.error = QStringLiteral("Failed to open '%1': %2").arg(path, localFile.errorString());
            return result;
        }

        const LoadResult loadedFile = loadSettingsFromData(localFile.readAll(), path, passwordProvider);
        result.settings = loadedFile.settings;
        result.error = loadedFile.error;
        result.warnings = loadedFile.warnings;
        result.ok = loadedFile.ok;
        return result;
    }

    if (url.scheme() != QStringLiteral("http") && url.scheme() != QStringLiteral("https")) {
        result.error = QStringLiteral("Unsupported configuration resource scheme: %1").arg(url.scheme());
        return result;
    }

    return settingsinternal::loadSettingsFromNetworkResource(url, passwordProvider);
}

}  // namespace seb
