#include "settings_mapping.h"

#include <QMetaType>
#include <QRegularExpression>
#include <QVariant>

namespace seb::settingsinternal {
namespace {

QString readString(const QVariantMap &data, const QString &key)
{
    const QVariant value = data.value(key);
    return value.isValid() ? value.toString() : QString();
}

bool readBool(const QVariantMap &data, const QString &key, bool fallback)
{
    const QVariant value = data.value(key);
    if (!value.isValid()) {
        return fallback;
    }
    if (value.metaType().id() == QMetaType::Bool || value.canConvert<bool>()) {
        return value.toBool();
    }

    const QString text = value.toString().trimmed().toLower();
    if (text == "true" || text == "yes" || text == "1") {
        return true;
    }
    if (text == "false" || text == "no" || text == "0") {
        return false;
    }
    return fallback;
}

int readInt(const QVariantMap &data, const QString &key, int fallback)
{
    const QVariant value = data.value(key);
    if (!value.isValid()) {
        return fallback;
    }
    if (value.canConvert<int>()) {
        return value.toInt();
    }
    bool ok = false;
    const int parsed = value.toString().trimmed().toInt(&ok);
    return ok ? parsed : fallback;
}

void applyDimension(WindowSettings &target, const QVariant &rawValue, bool width)
{
    if (!rawValue.isValid()) {
        return;
    }

    const QString text = rawValue.toString().trimmed();
    if (text.isEmpty()) {
        return;
    }

    bool ok = false;
    if (text.endsWith('%')) {
        const int relative = text.left(text.size() - 1).toInt(&ok);
        if (ok) {
            if (width) {
                target.relativeWidth = relative;
                target.absoluteWidth = 0;
            } else {
                target.relativeHeight = relative;
                target.absoluteHeight = 0;
            }
        }
        return;
    }

    const int absolute = text.toInt(&ok);
    if (ok) {
        if (width) {
            target.absoluteWidth = absolute;
            target.relativeWidth = 0;
        } else {
            target.absoluteHeight = absolute;
            target.relativeHeight = 0;
        }
    }
}

WindowPosition parseWindowPositionInt(int raw, WindowPosition fallback)
{
    switch (raw) {
    case 0: return WindowPosition::Left;
    case 1: return WindowPosition::Center;
    case 2: return WindowPosition::Right;
    default: return fallback;
    }
}

WindowPosition parseWindowPositionText(const QString &raw, WindowPosition fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "left") return WindowPosition::Left;
    if (normalized == "center" || normalized == "centre") return WindowPosition::Center;
    if (normalized == "right") return WindowPosition::Right;
    return fallback;
}

PopupPolicy parsePopupPolicyInt(int raw, bool blockForeignHost, PopupPolicy fallback)
{
    switch (raw) {
    case 2: return blockForeignHost ? PopupPolicy::AllowSameHost : PopupPolicy::Allow;
    case 1: return blockForeignHost ? PopupPolicy::AllowSameHostAndWindow : PopupPolicy::AllowSameWindow;
    case 0: return PopupPolicy::Block;
    default: return fallback;
    }
}

PopupPolicy parsePopupPolicyText(const QString &raw, PopupPolicy fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "allow") return PopupPolicy::Allow;
    if (normalized == "allowsamehost") return PopupPolicy::AllowSameHost;
    if (normalized == "allowsamehostandwindow") return PopupPolicy::AllowSameHostAndWindow;
    if (normalized == "allowsamewindow") return PopupPolicy::AllowSameWindow;
    if (normalized == "block") return PopupPolicy::Block;
    return fallback;
}

FilterResult parseFilterResultInt(int raw, FilterResult fallback)
{
    return raw == 0 ? FilterResult::Block : (raw == 1 ? FilterResult::Allow : fallback);
}

FilterResult parseFilterResultText(const QString &raw, FilterResult fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "allow") return FilterResult::Allow;
    if (normalized == "block") return FilterResult::Block;
    return fallback;
}

FilterRuleType parseFilterRuleTypeText(const QString &raw, FilterRuleType fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "regex") return FilterRuleType::Regex;
    if (normalized == "simplified") return FilterRuleType::Simplified;
    return fallback;
}

ProxyPolicy parseProxyPolicyInt(int raw, ProxyPolicy fallback)
{
    switch (raw) {
    case 1: return ProxyPolicy::Custom;
    case 0: return ProxyPolicy::System;
    default: return fallback;
    }
}

ProxyPolicy parseProxyPolicyText(const QString &raw, ProxyPolicy fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "custom") return ProxyPolicy::Custom;
    if (normalized == "system") return ProxyPolicy::System;
    return fallback;
}

ProxyProtocol parseProxyProtocolText(const QString &raw, ProxyProtocol fallback)
{
    const QString normalized = raw.trimmed().toLower();
    if (normalized == "ftp") return ProxyProtocol::Ftp;
    if (normalized == "http") return ProxyProtocol::Http;
    if (normalized == "https") return ProxyProtocol::Https;
    if (normalized == "socks") return ProxyProtocol::Socks;
    return fallback;
}

void applyStructuredWindowSettings(WindowSettings &target, const QVariantMap &data)
{
    target.allowAddressBar = readBool(data, "allowAddressBar", target.allowAddressBar);
    target.allowBackwardNavigation = readBool(data, "allowBackwardNavigation", target.allowBackwardNavigation);
    target.allowDeveloperConsole = readBool(data, "allowDeveloperConsole", target.allowDeveloperConsole);
    target.allowForwardNavigation = readBool(data, "allowForwardNavigation", target.allowForwardNavigation);
    target.allowMinimize = readBool(data, "allowMinimize", target.allowMinimize);
    target.allowReloading = readBool(data, "allowReloading", target.allowReloading);
    target.alwaysOnTop = readBool(data, "alwaysOnTop", target.alwaysOnTop);
    target.frameless = readBool(data, "frameless", target.frameless);
    target.fullScreenMode = readBool(data, "fullScreenMode", target.fullScreenMode);
    target.showHomeButton = readBool(data, "showHomeButton", target.showHomeButton);
    target.showReloadButton = readBool(data, "showReloadButton", target.showReloadButton);
    target.showReloadWarning = readBool(data, "showReloadWarning", target.showReloadWarning);
    target.showToolbar = readBool(data, "showToolbar", target.showToolbar);

    if (data.contains("absoluteWidth")) {
        target.absoluteWidth = readInt(data, "absoluteWidth", target.absoluteWidth);
        if (target.absoluteWidth > 0) target.relativeWidth = 0;
    }
    if (data.contains("absoluteHeight")) {
        target.absoluteHeight = readInt(data, "absoluteHeight", target.absoluteHeight);
        if (target.absoluteHeight > 0) target.relativeHeight = 0;
    }
    if (data.contains("relativeWidth")) {
        target.relativeWidth = readInt(data, "relativeWidth", target.relativeWidth);
        if (target.relativeWidth > 0) target.absoluteWidth = 0;
    }
    if (data.contains("relativeHeight")) {
        target.relativeHeight = readInt(data, "relativeHeight", target.relativeHeight);
        if (target.relativeHeight > 0) target.absoluteHeight = 0;
    }

    const QString position = readString(data, "position");
    if (!position.isEmpty()) {
        target.position = parseWindowPositionText(position, target.position);
    }
}

void applyStructuredFilter(FilterSettings &target, const QVariantMap &data)
{
    target.processMainRequests = readBool(data, "processMainRequests", target.processMainRequests);
    target.processContentRequests = readBool(data, "processContentRequests", target.processContentRequests);

    const QVariantList rules = data.value("rules").toList();
    for (const QVariant &rawRule : rules) {
        const QVariantMap ruleData = rawRule.toMap();
        const bool active = readBool(ruleData, "active", true);
        const QString expression = readString(ruleData, "expression").trimmed();
        if (!active || expression.isEmpty()) continue;

        FilterRuleSettings rule;
        rule.expression = expression;
        rule.result = parseFilterResultText(readString(ruleData, "result"), rule.result);
        rule.type = parseFilterRuleTypeText(readString(ruleData, "type"), rule.type);
        target.rules.push_back(rule);
    }
}

void applyStructuredProxy(ProxySettings &target, const QVariantMap &data)
{
    target.autoConfigure = readBool(data, "autoConfigure", target.autoConfigure);
    target.autoDetect = readBool(data, "autoDetect", target.autoDetect);
    target.autoConfigureUrl = readString(data, "autoConfigureUrl");
    target.policy = parseProxyPolicyText(readString(data, "policy"), target.policy);

    const QVariantList bypassList = data.value("bypassList").toList();
    for (const QVariant &item : bypassList) {
        const QString host = item.toString().trimmed();
        if (!host.isEmpty()) target.bypassList.push_back(host);
    }

    const QVariantList proxies = data.value("proxies").toList();
    for (const QVariant &entry : proxies) {
        const QVariantMap proxyData = entry.toMap();
        ProxyConfiguration proxy;
        proxy.host = readString(proxyData, "host").trimmed();
        proxy.password = readString(proxyData, "password");
        proxy.port = readInt(proxyData, "port", 0);
        proxy.protocol = parseProxyProtocolText(readString(proxyData, "protocol"), proxy.protocol);
        proxy.requiresAuthentication = readBool(proxyData, "requiresAuthentication", false);
        proxy.username = readString(proxyData, "username");
        if (!proxy.host.isEmpty() && proxy.port > 0) target.proxies.push_back(proxy);
    }
}

void mapRawFilterRules(FilterSettings &target, const QVariantList &rules)
{
    for (const QVariant &entry : rules) {
        const QVariantMap ruleData = entry.toMap();
        const bool active = readBool(ruleData, "active", false);
        const QString expression = readString(ruleData, "expression").trimmed();
        if (!active || expression.isEmpty()) continue;

        FilterRuleSettings rule;
        rule.expression = expression;
        rule.result = parseFilterResultInt(readInt(ruleData, "action", 0), rule.result);
        rule.type = readBool(ruleData, "regex", false) ? FilterRuleType::Regex : FilterRuleType::Simplified;
        target.rules.push_back(rule);
    }
}

void mapRawProxySettings(ProxySettings &target, const QVariantMap &proxyData)
{
    target.autoConfigure = readBool(proxyData, "AutoConfigurationEnabled", target.autoConfigure);
    target.autoConfigureUrl = readString(proxyData, "AutoConfigurationURL");
    target.autoDetect = readBool(proxyData, "AutoDiscoveryEnabled", target.autoDetect);

    const QVariantList bypassList = proxyData.value("ExceptionsList").toList();
    for (const QVariant &item : bypassList) {
        const QString host = item.toString().trimmed();
        if (!host.isEmpty()) target.bypassList.push_back(host);
    }

    auto appendProxy = [&](const QString &prefix, ProxyProtocol protocol) {
        if (!readBool(proxyData, prefix + "Enable", false)) return;

        ProxyConfiguration proxy;
        proxy.host = readString(proxyData, prefix + "Proxy").trimmed();
        proxy.password = readString(proxyData, prefix + "Password");
        proxy.port = readInt(proxyData, prefix + "Port", 0);
        proxy.protocol = protocol;
        proxy.requiresAuthentication = readBool(proxyData, prefix + "RequiresPassword", false);
        proxy.username = readString(proxyData, prefix + "Username");

        if (!proxy.host.isEmpty() && proxy.port > 0) target.proxies.push_back(proxy);
    };

    appendProxy(QStringLiteral("FTP"), ProxyProtocol::Ftp);
    appendProxy(QStringLiteral("HTTP"), ProxyProtocol::Http);
    appendProxy(QStringLiteral("HTTPS"), ProxyProtocol::Https);
    appendProxy(QStringLiteral("SOCKS"), ProxyProtocol::Socks);
}

}  // namespace

bool hasStructuredSections(const QVariantMap &data)
{
    return data.contains("browser") || data.contains("security");
}

QByteArray readBytes(const QVariant &value)
{
    if (!value.isValid()) {
        return {};
    }

    if (value.metaType().id() == QMetaType::QByteArray) {
        return value.toByteArray();
    }

    const QString text = value.toString().trimmed();
    if (text.isEmpty()) {
        return {};
    }

    const auto hexPattern = QRegularExpression(QStringLiteral("^[0-9a-fA-F]+$"));
    if (hexPattern.match(text).hasMatch() && text.size() % 2 == 0) {
        return QByteArray::fromHex(text.toUtf8());
    }

    return QByteArray::fromBase64(text.toUtf8());
}

void applyStructuredSettings(const QVariantMap &data, SebSettings &settings)
{
    const QVariantMap browser = data.value("browser").toMap();
    const QVariantMap security = data.value("security").toMap();

    if (!browser.isEmpty()) {
        const QString startUrl = readString(browser, "startUrl");
        if (!startUrl.isEmpty()) settings.browser.startUrl = startUrl;

        settings.browser.startUrlQuery = readString(browser, "startUrlQuery");
        settings.browser.homeNavigationMessage = readString(browser, "homeNavigationMessage");
        settings.browser.homeUrl = readString(browser, "homeUrl");
        settings.browser.quitUrl = readString(browser, "quitUrl");
        settings.browser.configurationKey = readString(browser, "configurationKey");
        settings.browser.customBrowserExamKey = readString(browser, "customBrowserExamKey");
        settings.browser.customUserAgent = readString(browser, "customUserAgent");
        settings.browser.downloadDirectory = readString(browser, "downloadDirectory");
        settings.browser.userAgentSuffix = readString(browser, "userAgentSuffix");

        settings.browser.allowConfigurationDownloads = readBool(browser, "allowConfigurationDownloads", settings.browser.allowConfigurationDownloads);
        settings.browser.allowCustomDownAndUploadLocation = readBool(browser, "allowCustomDownAndUploadLocation", settings.browser.allowCustomDownAndUploadLocation);
        settings.browser.allowDownloads = readBool(browser, "allowDownloads", settings.browser.allowDownloads);
        settings.browser.allowFind = readBool(browser, "allowFind", settings.browser.allowFind);
        settings.browser.allowPageZoom = readBool(browser, "allowPageZoom", settings.browser.allowPageZoom);
        settings.browser.allowPdfReader = readBool(browser, "allowPdfReader", settings.browser.allowPdfReader);
        settings.browser.allowPdfReaderToolbar = readBool(browser, "allowPdfReaderToolbar", settings.browser.allowPdfReaderToolbar);
        settings.browser.allowPrint = readBool(browser, "allowPrint", settings.browser.allowPrint);
        settings.browser.allowSpellChecking = readBool(browser, "allowSpellChecking", settings.browser.allowSpellChecking);
        settings.browser.allowUploads = readBool(browser, "allowUploads", settings.browser.allowUploads);
        settings.browser.confirmQuitUrl = readBool(browser, "confirmQuitUrl", settings.browser.confirmQuitUrl);
        settings.browser.deleteCacheOnShutdown = readBool(browser, "deleteCacheOnShutdown", settings.browser.deleteCacheOnShutdown);
        settings.browser.deleteCookiesOnShutdown = readBool(browser, "deleteCookiesOnShutdown", settings.browser.deleteCookiesOnShutdown);
        settings.browser.deleteCookiesOnStartup = readBool(browser, "deleteCookiesOnStartup", settings.browser.deleteCookiesOnStartup);
        settings.browser.enableBrowser = readBool(browser, "enableBrowser", settings.browser.enableBrowser);
        settings.browser.homeNavigationRequiresPassword = readBool(browser, "homeNavigationRequiresPassword", settings.browser.homeNavigationRequiresPassword);
        settings.browser.resetOnQuitUrl = readBool(browser, "resetOnQuitUrl", settings.browser.resetOnQuitUrl);
        settings.browser.sendBrowserExamKey = readBool(browser, "sendBrowserExamKey", settings.browser.sendBrowserExamKey);
        settings.browser.sendConfigurationKey = readBool(browser, "sendConfigurationKey", settings.browser.sendConfigurationKey);
        settings.browser.showFileSystemElementPath = readBool(browser, "showFileSystemElementPath", settings.browser.showFileSystemElementPath);
        settings.browser.useCustomUserAgent = readBool(browser, "useCustomUserAgent", settings.browser.useCustomUserAgent);
        settings.browser.useIsolatedClipboard = readBool(browser, "useIsolatedClipboard", settings.browser.useIsolatedClipboard);
        settings.browser.useQueryParameter = readBool(browser, "useQueryParameter", settings.browser.useQueryParameter);
        settings.browser.useStartUrlAsHomeUrl = readBool(browser, "useStartUrlAsHomeUrl", settings.browser.useStartUrlAsHomeUrl);
        settings.browser.useTemporaryDownAndUploadDirectory = readBool(browser, "useTemporaryDownAndUploadDirectory", settings.browser.useTemporaryDownAndUploadDirectory);

        if (browser.contains("browserExamKeySalt")) settings.browser.browserExamKeySalt = readBytes(browser.value("browserExamKeySalt"));
        if (browser.contains("popupPolicy")) settings.browser.popupPolicy = parsePopupPolicyText(readString(browser, "popupPolicy"), settings.browser.popupPolicy);

        applyStructuredWindowSettings(settings.browser.mainWindow, browser.value("mainWindow").toMap());
        applyStructuredWindowSettings(settings.browser.additionalWindow, browser.value("additionalWindow").toMap());
        applyStructuredFilter(settings.browser.filter, browser.value("filter").toMap());
        applyStructuredProxy(settings.browser.proxy, browser.value("proxy").toMap());
    }

    if (!security.isEmpty()) {
        settings.security.allowTermination = readBool(security, "allowTermination", settings.security.allowTermination);
        settings.security.quitPasswordHash = readString(security, "quitPasswordHash");
    }
}

void applyRawSettings(const QVariantMap &data, SebSettings &settings)
{
    const QString startUrl = readString(data, "startURL");
    if (!startUrl.isEmpty()) settings.browser.startUrl = startUrl;

    settings.browser.startUrlQuery = readString(data, "startURLQuery");
    settings.browser.homeNavigationMessage = readString(data, "restartExamText");
    settings.browser.homeUrl = readString(data, "restartExamURL");
    settings.browser.quitUrl = readString(data, "quitURL");
    settings.browser.downloadDirectory = readString(data, "downloadDirectoryWin");
    settings.browser.userAgentSuffix = readString(data, "browserUserAgent");

    settings.browser.allowConfigurationDownloads = readBool(data, "downloadAndOpenSebConfig", settings.browser.allowConfigurationDownloads);
    settings.browser.allowCustomDownAndUploadLocation = readBool(data, "allowCustomDownUploadLocation", settings.browser.allowCustomDownAndUploadLocation);
    settings.browser.allowDownloads = readBool(data, "allowDownloads", settings.browser.allowDownloads);
    settings.browser.allowFind = readBool(data, "allowFind", settings.browser.allowFind);
    settings.browser.allowPageZoom = readBool(data, "enableZoomPage", settings.browser.allowPageZoom);
    settings.browser.allowPdfReaderToolbar = readBool(data, "allowPDFReaderToolbar", settings.browser.allowPdfReaderToolbar);
    settings.browser.allowPrint = readBool(data, "allowPrint", settings.browser.allowPrint);
    settings.browser.allowSpellChecking = readBool(data, "allowSpellCheck", settings.browser.allowSpellChecking);
    settings.browser.allowUploads = readBool(data, "allowUploads", settings.browser.allowUploads);
    settings.browser.confirmQuitUrl = readBool(data, "quitURLConfirm", settings.browser.confirmQuitUrl);
    settings.browser.deleteCacheOnShutdown = readBool(data, "removeBrowserProfile", settings.browser.deleteCacheOnShutdown);
    settings.browser.deleteCookiesOnShutdown = readBool(data, "examSessionClearCookiesOnEnd", settings.browser.deleteCookiesOnShutdown);
    settings.browser.deleteCookiesOnStartup = readBool(data, "examSessionClearCookiesOnStart", settings.browser.deleteCookiesOnStartup);
    settings.browser.enableBrowser = readBool(data, "enableSebBrowser", settings.browser.enableBrowser);
    settings.browser.homeNavigationRequiresPassword = readBool(data, "restartExamPasswordProtected", settings.browser.homeNavigationRequiresPassword);
    settings.browser.resetOnQuitUrl = readBool(data, "quitURLRestart", settings.browser.resetOnQuitUrl);
    settings.browser.showFileSystemElementPath = readBool(data, "browserShowFileSystemElementPath", settings.browser.showFileSystemElementPath);
    settings.browser.useQueryParameter = readBool(data, "startURLAppendQueryParameter", settings.browser.useQueryParameter);
    settings.browser.useStartUrlAsHomeUrl = readBool(data, "restartExamUseStartURL", settings.browser.useStartUrlAsHomeUrl);
    settings.browser.useTemporaryDownAndUploadDirectory = readBool(data, "useTemporaryDownUploadDirectory", settings.browser.useTemporaryDownAndUploadDirectory);

    if (readBool(data, "downloadPDFFiles", false)) settings.browser.allowPdfReader = false;

    const bool sendHeaders = readBool(data, "sendBrowserExamKey", false);
    settings.browser.sendConfigurationKey = sendHeaders;
    settings.browser.sendBrowserExamKey = sendHeaders;

    if (data.contains("examKeySalt")) settings.browser.browserExamKeySalt = readBytes(data.value("examKeySalt"));

    settings.browser.mainWindow.allowAddressBar = readBool(data, "browserWindowAllowAddressBar", settings.browser.mainWindow.allowAddressBar);
    settings.browser.mainWindow.allowReloading = readBool(data, "browserWindowAllowReload", settings.browser.mainWindow.allowReloading);
    settings.browser.mainWindow.showReloadButton = readBool(data, "showReloadButton", settings.browser.mainWindow.showReloadButton);
    settings.browser.mainWindow.showReloadWarning = readBool(data, "showReloadWarning", settings.browser.mainWindow.showReloadWarning);
    settings.browser.mainWindow.showToolbar = readBool(data, "enableBrowserWindowToolbar", settings.browser.mainWindow.showToolbar);

    const bool allowNavigation = readBool(data, "allowBrowsingBackForward", settings.browser.mainWindow.allowBackwardNavigation);
    settings.browser.mainWindow.allowBackwardNavigation = allowNavigation;
    settings.browser.mainWindow.allowForwardNavigation = allowNavigation;

    const bool allowDeveloperConsole = readBool(data, "allowDeveloperConsole", settings.browser.mainWindow.allowDeveloperConsole);
    settings.browser.mainWindow.allowDeveloperConsole = allowDeveloperConsole;
    settings.browser.additionalWindow.allowDeveloperConsole = allowDeveloperConsole;

    settings.browser.additionalWindow.allowAddressBar = readBool(data, "newBrowserWindowAllowAddressBar", settings.browser.additionalWindow.allowAddressBar);
    settings.browser.additionalWindow.allowReloading = readBool(data, "newBrowserWindowAllowReload", settings.browser.additionalWindow.allowReloading);
    settings.browser.additionalWindow.showReloadWarning = readBool(data, "newBrowserWindowShowReloadWarning", settings.browser.additionalWindow.showReloadWarning);

    const bool allowAdditionalNavigation = readBool(data, "newBrowserWindowNavigation", settings.browser.additionalWindow.allowBackwardNavigation);
    settings.browser.additionalWindow.allowBackwardNavigation = allowAdditionalNavigation;
    settings.browser.additionalWindow.allowForwardNavigation = allowAdditionalNavigation;

    const bool blockForeignHost = readBool(data, "newBrowserWindowByLinkBlockForeign", false);
    if (data.contains("newBrowserWindowByLinkPolicy")) {
        settings.browser.popupPolicy = parsePopupPolicyInt(readInt(data, "newBrowserWindowByLinkPolicy", 2), blockForeignHost, settings.browser.popupPolicy);
    }

    if (data.contains("browserViewMode")) {
        settings.browser.mainWindow.fullScreenMode = readInt(data, "browserViewMode", 0) == 1;
    }

    if (data.contains("mainBrowserWindowWidth")) applyDimension(settings.browser.mainWindow, data.value("mainBrowserWindowWidth"), true);
    if (data.contains("mainBrowserWindowHeight")) applyDimension(settings.browser.mainWindow, data.value("mainBrowserWindowHeight"), false);
    if (data.contains("newBrowserWindowByLinkWidth")) applyDimension(settings.browser.additionalWindow, data.value("newBrowserWindowByLinkWidth"), true);
    if (data.contains("newBrowserWindowByLinkHeight")) applyDimension(settings.browser.additionalWindow, data.value("newBrowserWindowByLinkHeight"), false);

    if (data.contains("mainBrowserWindowPositioning")) {
        settings.browser.mainWindow.position = parseWindowPositionInt(readInt(data, "mainBrowserWindowPositioning", 1), settings.browser.mainWindow.position);
    }
    if (data.contains("newBrowserWindowByLinkPositioning")) {
        settings.browser.additionalWindow.position = parseWindowPositionInt(readInt(data, "newBrowserWindowByLinkPositioning", 2), settings.browser.additionalWindow.position);
    }

    if (data.contains("browserUserAgentWinDesktopMode") && readInt(data, "browserUserAgentWinDesktopMode", 0) != 0) {
        settings.browser.useCustomUserAgent = true;
        settings.browser.customUserAgent = readString(data, "browserUserAgentWinDesktopModeCustom");
    }

    settings.browser.filter.processMainRequests = readBool(data, "URLFilterEnable", settings.browser.filter.processMainRequests);
    settings.browser.filter.processContentRequests = readBool(data, "URLFilterEnableContentFilter", settings.browser.filter.processContentRequests);
    mapRawFilterRules(settings.browser.filter, data.value("URLFilterRules").toList());

    settings.browser.proxy.policy = parseProxyPolicyInt(readInt(data, "proxySettingsPolicy", 0), settings.browser.proxy.policy);
    mapRawProxySettings(settings.browser.proxy, data.value("proxies").toMap());

    settings.security.allowTermination = readBool(data, "allowQuit", settings.security.allowTermination);
    settings.security.quitPasswordHash = readString(data, "hashedQuitPassword");
}

}  // namespace seb::settingsinternal
