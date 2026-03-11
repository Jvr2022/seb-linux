#include "settings_defaults.h"

namespace seb::settingsinternal {
namespace {

QString trimTrailingSlash(QString value)
{
    while (value.endsWith('/') && value.size() > 1) {
        value.chop(1);
    }
    return value;
}

}  // namespace

SebSettings createDefaultSettings()
{
    SebSettings settings;

    settings.browser.additionalWindow.allowAddressBar = false;
    settings.browser.additionalWindow.allowBackwardNavigation = true;
    settings.browser.additionalWindow.allowDeveloperConsole = false;
    settings.browser.additionalWindow.allowForwardNavigation = true;
    settings.browser.additionalWindow.allowReloading = true;
    settings.browser.additionalWindow.position = WindowPosition::Right;
    settings.browser.additionalWindow.relativeHeight = 100;
    settings.browser.additionalWindow.relativeWidth = 50;
    settings.browser.additionalWindow.showHomeButton = false;
    settings.browser.additionalWindow.showReloadButton = true;
    settings.browser.additionalWindow.showReloadWarning = false;
    settings.browser.additionalWindow.showToolbar = false;

    settings.browser.mainWindow.allowAddressBar = false;
    settings.browser.mainWindow.allowBackwardNavigation = false;
    settings.browser.mainWindow.allowDeveloperConsole = false;
    settings.browser.mainWindow.allowForwardNavigation = false;
    settings.browser.mainWindow.allowReloading = true;
    settings.browser.mainWindow.position = WindowPosition::Center;
    settings.browser.mainWindow.relativeHeight = 100;
    settings.browser.mainWindow.relativeWidth = 100;
    settings.browser.mainWindow.showHomeButton = false;
    settings.browser.mainWindow.showReloadButton = true;
    settings.browser.mainWindow.showReloadWarning = true;
    settings.browser.mainWindow.showToolbar = false;

    settings.browser.allowConfigurationDownloads = true;
    settings.browser.allowCustomDownAndUploadLocation = false;
    settings.browser.allowDownloads = true;
    settings.browser.allowFind = true;
    settings.browser.allowPageZoom = true;
    settings.browser.allowPdfReader = true;
    settings.browser.allowPdfReaderToolbar = false;
    settings.browser.allowPrint = false;
    settings.browser.allowSpellChecking = false;
    settings.browser.allowUploads = false;
    settings.browser.confirmQuitUrl = false;
    settings.browser.deleteCacheOnShutdown = true;
    settings.browser.deleteCookiesOnShutdown = true;
    settings.browser.deleteCookiesOnStartup = true;
    settings.browser.enableBrowser = true;
    settings.browser.popupPolicy = PopupPolicy::Allow;
    settings.browser.proxy.policy = ProxyPolicy::System;
    settings.browser.resetOnQuitUrl = false;
    settings.browser.showFileSystemElementPath = true;
    settings.browser.startUrl = QStringLiteral("https://www.safeexambrowser.org/start");
    settings.browser.useCustomUserAgent = false;
    settings.browser.useIsolatedClipboard = true;
    settings.browser.useQueryParameter = false;
    settings.browser.useStartUrlAsHomeUrl = false;
    settings.browser.useTemporaryDownAndUploadDirectory = false;

    settings.security.allowTermination = true;
    return settings;
}

void finalizeSettings(SebSettings &settings)
{
    settings.browser.startUrl = trimTrailingSlash(settings.browser.startUrl.trimmed());
    settings.browser.homeUrl = trimTrailingSlash(settings.browser.homeUrl.trimmed());
    settings.browser.quitUrl = trimTrailingSlash(settings.browser.quitUrl.trimmed());
    settings.browser.configurationKey = settings.browser.configurationKey.trimmed();
    settings.browser.customBrowserExamKey = settings.browser.customBrowserExamKey.trimmed();
    settings.browser.customUserAgent = settings.browser.customUserAgent.trimmed();
    settings.security.quitPasswordHash = settings.security.quitPasswordHash.trimmed().toLower();

    if (settings.browser.useStartUrlAsHomeUrl && settings.browser.homeUrl.isEmpty()) {
        settings.browser.homeUrl = settings.browser.startUrl;
    }

    if (!settings.browser.homeUrl.isEmpty()) {
        settings.browser.mainWindow.showHomeButton = true;
    }

    if ((settings.browser.mainWindow.allowReloading && settings.browser.mainWindow.showReloadButton) ||
        settings.browser.mainWindow.allowAddressBar) {
        settings.browser.mainWindow.showToolbar = true;
    }

    if ((settings.browser.additionalWindow.allowReloading && settings.browser.additionalWindow.showReloadButton) ||
        settings.browser.additionalWindow.allowAddressBar) {
        settings.browser.additionalWindow.showToolbar = true;
    }

    if (settings.browser.mainWindow.fullScreenMode) {
        settings.browser.mainWindow.alwaysOnTop = true;
    }
}

}  // namespace seb::settingsinternal
