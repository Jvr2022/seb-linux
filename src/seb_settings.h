#pragma once

#include <QByteArray>
#include <QList>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <functional>

namespace seb {

enum class PopupPolicy
{
    Allow,
    AllowSameHost,
    AllowSameHostAndWindow,
    AllowSameWindow,
    Block
};

enum class WindowPosition
{
    Left,
    Center,
    Right
};

enum class FilterResult
{
    Allow,
    Block
};

enum class FilterRuleType
{
    Regex,
    Simplified
};

enum class ProxyPolicy
{
    Custom,
    System
};

enum class ProxyProtocol
{
    Ftp,
    Http,
    Https,
    Socks
};

struct WindowSettings
{
    int absoluteHeight = 0;
    int absoluteWidth = 0;
    int relativeHeight = 100;
    int relativeWidth = 100;
    bool allowAddressBar = false;
    bool allowBackwardNavigation = false;
    bool allowDeveloperConsole = false;
    bool allowForwardNavigation = false;
    bool allowMinimize = true;
    bool allowReloading = true;
    bool alwaysOnTop = false;
    bool frameless = false;
    bool fullScreenMode = false;
    bool showHomeButton = false;
    bool showReloadButton = false;
    bool showReloadWarning = false;
    bool showToolbar = false;
    WindowPosition position = WindowPosition::Center;
};

struct FilterRuleSettings
{
    QString expression;
    FilterResult result = FilterResult::Block;
    FilterRuleType type = FilterRuleType::Simplified;
};

struct FilterSettings
{
    bool processContentRequests = false;
    bool processMainRequests = false;
    QList<FilterRuleSettings> rules;
};

struct ProxyConfiguration
{
    QString host;
    QString password;
    QString username;
    int port = 0;
    ProxyProtocol protocol = ProxyProtocol::Http;
    bool requiresAuthentication = false;
};

struct ProxySettings
{
    bool autoConfigure = false;
    bool autoDetect = false;
    QString autoConfigureUrl;
    QStringList bypassList;
    QList<ProxyConfiguration> proxies;
    ProxyPolicy policy = ProxyPolicy::System;
};

struct BrowserSettings
{
    WindowSettings additionalWindow;
    WindowSettings mainWindow;
    FilterSettings filter;
    ProxySettings proxy;
    QByteArray browserExamKeySalt;
    QString configurationKey;
    QString customBrowserExamKey;
    QString customUserAgent;
    QString downloadDirectory;
    QString homeNavigationMessage;
    QString homeUrl;
    QString quitUrl;
    QString startUrl;
    QString startUrlQuery;
    QString userAgentSuffix;
    QString injectedScript;
    bool allowConfigurationDownloads = true;
    bool allowCustomDownAndUploadLocation = false;
    bool allowDownloads = true;
    bool allowFind = true;
    bool allowPageZoom = true;
    bool allowPdfReader = true;
    bool allowPdfReaderToolbar = false;
    bool allowPrint = false;
    bool allowSpellChecking = false;
    bool allowUploads = false;
    bool confirmQuitUrl = false;
    bool deleteCacheOnShutdown = true;
    bool deleteCookiesOnShutdown = true;
    bool deleteCookiesOnStartup = true;
    bool enableBrowser = true;
    bool homeNavigationRequiresPassword = false;
    bool resetOnQuitUrl = false;
    bool sendBrowserExamKey = false;
    bool sendConfigurationKey = false;
    bool showFileSystemElementPath = true;
    bool useCustomUserAgent = false;
    bool useIsolatedClipboard = true;
    bool useQueryParameter = false;
    bool useStartUrlAsHomeUrl = false;
    bool useTemporaryDownAndUploadDirectory = false;
    PopupPolicy popupPolicy = PopupPolicy::Allow;
};

struct TaskbarSettings
{
    bool enableTaskbar = true;
    bool showApplicationInfo = false;
    bool showApplicationLog = false;
    bool showAudio = true;
    bool showClock = true;
    bool showKeyboardLayout = true;
    bool showNetwork = false;
    bool showVerificator = true;
    bool showProctoringNotification = true;
};

struct BlacklistApplicationSettings
{
    bool autoTerminate = false;
    QString executableName;
    QString originalName;
};

struct WhitelistApplicationSettings
{
    bool allowCustomPath = false;
    bool allowRunning = false;
    QStringList arguments;
    bool autoStart = false;
    bool autoTerminate = false;
    QString description;
    QString displayName;
    QString executableName;
    QString executablePath;
    QString id;
    QString originalName;
    bool showInShell = true;
    QString signature;
};

struct ApplicationSettings
{
    QList<BlacklistApplicationSettings> blacklist;
    QList<WhitelistApplicationSettings> whitelist;
};

struct SecuritySettings
{
    bool allowTermination = true;
    QString quitPasswordHash;
};

struct SebSettings
{
    ApplicationSettings applications;
    BrowserSettings browser;
    SecuritySettings security;
    TaskbarSettings taskbar;
    QString sourceFile;
};

struct LoadResult
{
    SebSettings settings;
    QString error;
    QStringList warnings;
    bool ok = false;
};

struct ResourceLoadResult
{
    SebSettings settings;
    QUrl browserUrl;
    QString error;
    QStringList warnings;
    bool ok = false;
    bool openInBrowser = false;
};

SebSettings defaultSettings();
LoadResult loadSettingsFromFile(const QString &path);
LoadResult loadSettingsFromData(
    const QByteArray &raw,
    const QString &sourceName = {},
    const std::function<QString (bool)> &passwordProvider = {});
ResourceLoadResult loadSettingsFromResource(
    const QString &resource,
    const std::function<QString (bool)> &passwordProvider = {});

}  // namespace seb
