#pragma once

#include <QDateTime>
#include <QString>
#include <QUuid>

namespace seb::configuration::contracts {

struct AppConfig
{
    static inline constexpr auto BackupFileName = "Backup.bin";
    static inline constexpr auto BaseAddress = "local://safeexambrowser";
    static inline constexpr auto ClientMutexName = "safe_exam_browser_client_mutex";
    static inline constexpr auto RuntimeMutexName = "safe_exam_browser_runtime_mutex";
    static inline constexpr auto ServiceAddress = "local://safeexambrowser/service";
    static inline constexpr auto ServiceMutexName = "safe_exam_browser_reset_mutex";

    QString appDataFilePath;
    QDateTime applicationStartTime;
    QString browserCachePath;
    QString browserLogFilePath;
    QString clientAddress;
    QString clientExecutablePath;
    QUuid clientId;
    QString clientLogFilePath;
    QString codeSignatureHash;
    QString configurationFileExtension;
    QString configurationFileMimeType;
    QString programBuildVersion;
    QString programCopyright;
    QString programDataFilePath;
    QString programTitle;
    QString programInformationalVersion;
    QString runtimeAddress;
    QUuid runtimeId;
    QString runtimeLogFilePath;
    QString sebUriScheme;
    QString sebUriSchemeSecure;
    QString serverApi;
    QString serverConnectionToken;
    QString serverExamId;
    QString serverOauth2Token;
    QString serviceAddress;
    QString serviceEventName;
    QString serviceLogFilePath;
    QString sessionCacheFilePath;
    QString temporaryDirectory;
};

}  // namespace seb::configuration::contracts
