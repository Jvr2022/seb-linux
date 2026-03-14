#pragma once

#include "FilterSettings.h"
#include "PopupPolicy.h"
#include "ProxySettings.h"
#include "WindowSettings.h"

#include <QByteArray>
#include <QString>

namespace seb::settings::browser {

struct BrowserSettings
{
    WindowSettings AdditionalWindow;
    bool AllowConfigurationDownloads = true;
    bool AllowCustomDownAndUploadLocation = false;
    bool AllowDownloads = true;
    bool AllowFind = true;
    bool AllowPageZoom = true;
    bool AllowPdfReader = true;
    bool AllowPdfReaderToolbar = false;
    bool AllowPrint = false;
    bool AllowSpellChecking = false;
    bool AllowUploads = false;
    QByteArray BrowserExamKeySalt;
    QString ConfigurationKey;
    bool ConfirmQuitUrl = false;
    QString CustomBrowserExamKey;
    QString CustomUserAgent;
    bool DeleteCacheOnShutdown = true;
    bool DeleteCookiesOnShutdown = true;
    bool DeleteCookiesOnStartup = true;
    QString DownAndUploadDirectory;
    bool EnableBrowser = true;
    FilterSettings Filter;
    QString HomeNavigationMessage;
    bool HomeNavigationRequiresPassword = false;
    QString HomePasswordHash;
    QString HomeUrl;
    WindowSettings MainWindow;
    PopupPolicy PopupPolicy = PopupPolicy::All;
    ProxySettings Proxy;
    QString QuitUrl;
    bool ResetOnQuitUrl = false;
    bool SendConfigurationKey = false;
    bool SendBrowserExamKey = false;
    bool ShowFileSystemElementPath = true;
    QString StartUrl;
    QString StartUrlQuery;
    bool UseCustomUserAgent = false;
    bool UseIsolatedClipboard = true;
    bool UseQueryParameter = false;
    QString UserAgentSuffix;
    bool UseStartUrlAsHomeUrl = false;
    bool UseTemporaryDownAndUploadDirectory = false;
};

}  // namespace seb::settings::browser
