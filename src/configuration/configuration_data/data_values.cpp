#include "data_values.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

namespace seb::configuration::configuration_data {

contracts::AppConfig DataValues::initializeAppConfig() const
{
    contracts::AppConfig config;
    const QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString tempDir = QDir::tempPath() + QStringLiteral("/seb-linux");

    config.applicationStartTime = QDateTime::currentDateTimeUtc();
    config.appDataFilePath = appData + QStringLiteral("/client.seb");
    config.browserCachePath = appData + QStringLiteral("/cache");
    config.browserLogFilePath = appData + QStringLiteral("/browser.log");
    config.clientAddress = QStringLiteral("local://safeexambrowser/client");
    config.clientExecutablePath = QCoreApplication::applicationFilePath();
    config.clientId = QUuid::createUuid();
    config.clientLogFilePath = appData + QStringLiteral("/client.log");
    config.configurationFileExtension = QStringLiteral(".seb");
    config.configurationFileMimeType = QStringLiteral("application/x-seb");
    config.programBuildVersion = QStringLiteral("1.0.0");
    config.programCopyright = QStringLiteral("JVR2022");
    config.programDataFilePath = appData + QStringLiteral("/program.seb");
    config.programTitle = QStringLiteral("Safe Exam Browser");
    config.programInformationalVersion = QStringLiteral("1.0.0");
    config.runtimeAddress = QStringLiteral("local://safeexambrowser/runtime");
    config.runtimeId = QUuid::createUuid();
    config.runtimeLogFilePath = appData + QStringLiteral("/runtime.log");
    config.sebUriScheme = QStringLiteral("seb");
    config.sebUriSchemeSecure = QStringLiteral("sebs");
    config.serviceAddress = QStringLiteral("local://safeexambrowser/service");
    config.serviceLogFilePath = appData + QStringLiteral("/service.log");
    config.sessionCacheFilePath = appData + QStringLiteral("/session.cache");
    config.temporaryDirectory = tempDir;
    return config;
}

contracts::SessionConfiguration DataValues::initializeSessionConfiguration() const
{
    contracts::SessionConfiguration configuration;
    configuration.appConfig = initializeAppConfig();
    configuration.clientAuthenticationToken = QUuid::createUuid();
    configuration.sessionId = QUuid::createUuid();
    configuration.settings = loadDefaultSettings();
    return configuration;
}

seb::SebSettings DataValues::loadDefaultSettings() const
{
    return seb::defaultSettings();
}

QString DataValues::getAppDataFilePath() const
{
    return initializeAppConfig().appDataFilePath;
}

}  // namespace seb::configuration::configuration_data
