#include "browser_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {

namespace {
using namespace helpers;

void mapWindow(seb::settings::browser::WindowSettings &window, const QString &key, const QVariant &value, bool additional)
{
    const QString prefix = additional ? QStringLiteral("newBrowserWindow") : QStringLiteral("browserWindow");
    if (key == prefix + QStringLiteral("AllowAddressBar")) window.AllowAddressBar = boolean(value, window.AllowAddressBar);
    else if (key == prefix + QStringLiteral("AllowNavigation") || (!additional && key == keys::Browser::MainWindow::AllowNavigation) || (additional && key == keys::Browser::AdditionalWindow::AllowNavigation)) { window.AllowBackwardNavigation = boolean(value, window.AllowBackwardNavigation); window.AllowForwardNavigation = window.AllowBackwardNavigation; }
    else if (key == prefix + QStringLiteral("AllowReload")) window.AllowReloading = boolean(value, window.AllowReloading);
    else if (key == prefix + QStringLiteral("AllowMinimize")) window.AllowMinimize = boolean(value, window.AllowMinimize);
    else if (key == prefix + QStringLiteral("AllowDeveloperConsole")) window.AllowDeveloperConsole = boolean(value, window.AllowDeveloperConsole);
    else if (key == prefix + QStringLiteral("AlwaysOnTop")) window.AlwaysOnTop = boolean(value, window.AlwaysOnTop);
    else if (key == prefix + QStringLiteral("Fullscreen") || (!additional && key == keys::Browser::MainWindow::WindowMode)) window.FullScreenMode = integer(value) == 1;
    else if (key == prefix + QStringLiteral("ShowHomeButton")) window.ShowHomeButton = boolean(value, window.ShowHomeButton);
    else if (key == prefix + QStringLiteral("ShowMenuBar")) window.ShowMenuBar = boolean(value, window.ShowMenuBar);
    else if (key == prefix + QStringLiteral("ShowReloadButton")) window.ShowReloadButton = boolean(value, window.ShowReloadButton);
    else if (key == prefix + QStringLiteral("ShowReloadWarning")) window.ShowReloadWarning = boolean(value, window.ShowReloadWarning);
    else if (key == prefix + QStringLiteral("ShowToolbar")) window.ShowToolbar = boolean(value, window.ShowToolbar);
    else if (key == prefix + QStringLiteral("ShowTitleBar")) window.ShowTitleBar = boolean(value, window.ShowTitleBar);
    else if (key == prefix + QStringLiteral("Height") || (!additional && key == keys::Browser::MainWindow::WindowHeight) || (additional && key == keys::Browser::AdditionalWindow::WindowHeight)) dimension(value, window.AbsoluteHeight, window.RelativeHeight);
    else if (key == prefix + QStringLiteral("Width") || (!additional && key == keys::Browser::MainWindow::WindowWidth) || (additional && key == keys::Browser::AdditionalWindow::WindowWidth)) dimension(value, window.AbsoluteWidth, window.RelativeWidth);
    else if (key == (!additional ? keys::Browser::MainWindow::WindowPosition : keys::Browser::AdditionalWindow::WindowPosition)) window.Position = static_cast<seb::settings::browser::WindowPosition>(integer(value, static_cast<int>(window.Position)));
    else if (key == (!additional ? keys::Browser::MainWindow::UrlPolicy : keys::Browser::AdditionalWindow::UrlPolicy)) {
        switch (integer(value)) {
        case 0: window.UrlPolicy = seb::settings::browser::UrlPolicy::Never; break;
        case 1: window.UrlPolicy = seb::settings::browser::UrlPolicy::LoadError; break;
        case 2: window.UrlPolicy = seb::settings::browser::UrlPolicy::BeforeTitle; break;
        case 3: window.UrlPolicy = seb::settings::browser::UrlPolicy::Always; break;
        }
    }
}

void mapFilter(seb::settings::browser::FilterSettings &filter, const QVariant &value)
{
    for (const QVariant &entry : value.toList()) {
        const QVariantMap raw = entry.toMap();
        seb::settings::browser::filter::FilterRuleSettings rule;
        rule.Active = boolean(raw.value(QStringLiteral("active")), true);
        rule.Expression = string(raw.value(QStringLiteral("expression")));
        rule.Result = integer(raw.value(QStringLiteral("action"))) == 1 ? seb::settings::browser::filter::FilterResult::Allow : seb::settings::browser::filter::FilterResult::Block;
        rule.Type = boolean(raw.value(QStringLiteral("regex"))) ? seb::settings::browser::filter::FilterRuleType::Regex : seb::settings::browser::filter::FilterRuleType::Simplified;
        if (rule.Active && !rule.Expression.isEmpty()) filter.Rules.push_back(rule);
    }
}
}

void BrowserDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &browser = settings.Browser;
    if (key == keys::Browser::AllowConfigurationDownloads) browser.AllowConfigurationDownloads = boolean(value, browser.AllowConfigurationDownloads);
    else if (key == keys::Browser::AllowCustomDownUploadLocation) browser.AllowCustomDownAndUploadLocation = boolean(value, browser.AllowCustomDownAndUploadLocation);
    else if (key == keys::Browser::AllowDownloads) browser.AllowDownloads = boolean(value, browser.AllowDownloads);
    else if (key == keys::Browser::AllowFind) browser.AllowFind = boolean(value, browser.AllowFind);
    else if (key == keys::Browser::AllowPageZoom) browser.AllowPageZoom = boolean(value, browser.AllowPageZoom);
    else if (key == keys::Browser::AllowDeveloperConsole) { browser.MainWindow.AllowDeveloperConsole = boolean(value, browser.MainWindow.AllowDeveloperConsole); browser.AdditionalWindow.AllowDeveloperConsole = browser.MainWindow.AllowDeveloperConsole; }
    else if (key == keys::Browser::AllowPdfReaderToolbar) browser.AllowPdfReaderToolbar = boolean(value, browser.AllowPdfReaderToolbar);
    else if (key == keys::Browser::AllowPrint) browser.AllowPrint = boolean(value, browser.AllowPrint);
    else if (key == keys::Browser::AllowSpellChecking) browser.AllowSpellChecking = boolean(value, browser.AllowSpellChecking);
    else if (key == keys::Browser::AllowUploads) browser.AllowUploads = boolean(value, browser.AllowUploads);
    else if (key == keys::Browser::ExamKeySalt) browser.BrowserExamKeySalt = value.toByteArray();
    else if (key == keys::Browser::CustomUserAgentDesktop) browser.CustomUserAgent = string(value);
    else if (key == keys::Browser::DeleteCacheOnShutdown) browser.DeleteCacheOnShutdown = boolean(value, browser.DeleteCacheOnShutdown);
    else if (key == keys::Browser::DeleteCookiesOnShutdown) browser.DeleteCookiesOnShutdown = boolean(value, browser.DeleteCookiesOnShutdown);
    else if (key == keys::Browser::DeleteCookiesOnStartup) browser.DeleteCookiesOnStartup = boolean(value, browser.DeleteCookiesOnStartup);
    else if (key == keys::Browser::DownloadDirectory) browser.DownAndUploadDirectory = string(value);
    else if (key == keys::Browser::EnableBrowser) browser.EnableBrowser = boolean(value, browser.EnableBrowser);
    else if (key == keys::Browser::Filter::FilterRules) mapFilter(browser.Filter, value);
    else if (key == keys::Browser::HomeButtonMessage) browser.HomeNavigationMessage = string(value);
    else if (key == keys::Browser::HomeButtonRequiresPassword) browser.HomeNavigationRequiresPassword = boolean(value, browser.HomeNavigationRequiresPassword);
    else if (key == keys::Browser::HomeButtonUrl) browser.HomeUrl = string(value);
    else if (key == keys::Browser::QuitUrl) browser.QuitUrl = string(value);
    else if (key == keys::Browser::QuitUrlConfirmation) browser.ConfirmQuitUrl = boolean(value, browser.ConfirmQuitUrl);
    else if (key == keys::Browser::ResetOnQuitUrl) browser.ResetOnQuitUrl = boolean(value, browser.ResetOnQuitUrl);
    else if (key == keys::Browser::SendCustomHeaders) { browser.SendBrowserExamKey = boolean(value, browser.SendBrowserExamKey); browser.SendConfigurationKey = browser.SendBrowserExamKey; }
    else if (key == keys::Browser::ShowFileSystemElementPath) browser.ShowFileSystemElementPath = boolean(value, browser.ShowFileSystemElementPath);
    else if (key == keys::Browser::StartUrl) browser.StartUrl = string(value);
    else if (key == keys::Browser::UseStartUrlQuery) { browser.UseQueryParameter = boolean(value, browser.UseQueryParameter); browser.StartUrlQuery = string(value); }
    else if (key == keys::Browser::UserAgentSuffix) browser.UserAgentSuffix = string(value);
    else if (key == keys::Browser::UserAgentModeDesktop) browser.UserAgentMode = string(value);
    else if (key == keys::Browser::HomeButtonUseStartUrl) browser.UseStartUrlAsHomeUrl = boolean(value, browser.UseStartUrlAsHomeUrl);
    else if (key == keys::Browser::UseTemporaryDownUploadDirectory) browser.UseTemporaryDownAndUploadDirectory = boolean(value, browser.UseTemporaryDownAndUploadDirectory);
    else if (key == keys::Browser::ShowReloadButton) { browser.MainWindow.ShowReloadButton = boolean(value, browser.MainWindow.ShowReloadButton); browser.AdditionalWindow.ShowReloadButton = browser.MainWindow.ShowReloadButton; }
    else if (key == keys::Browser::ShowToolbar) { browser.MainWindow.ShowToolbar = boolean(value, browser.MainWindow.ShowToolbar); browser.AdditionalWindow.ShowToolbar = browser.MainWindow.ShowToolbar; }
    else if (key == keys::Browser::Filter::EnableContentRequestFilter) browser.Filter.ProcessContentRequests = boolean(value, browser.Filter.ProcessContentRequests);
    else if (key == keys::Browser::Filter::EnableMainRequestFilter) browser.Filter.ProcessMainRequests = boolean(value, browser.Filter.ProcessMainRequests);
    else if (key == keys::Browser::Proxy::Policy) browser.Proxy.Policy = integer(value) == 1 ? seb::settings::browser::proxy::ProxyPolicy::Custom : seb::settings::browser::proxy::ProxyPolicy::System;
    else if (key == keys::Browser::Proxy::Settings) {
        const QVariantMap proxy = value.toMap();
        browser.Proxy.AutoConfigure = boolean(proxy.value(keys::Browser::Proxy::AutoConfigure));
        browser.Proxy.AutoDetect = boolean(proxy.value(keys::Browser::Proxy::AutoDetect));
        browser.Proxy.AutoConfigureUrl = string(proxy.value(keys::Browser::Proxy::AutoConfigureUrl));
        browser.Proxy.BypassList = strings(proxy.value(keys::Browser::Proxy::BypassList));
        const auto appendProxy = [&](const QString &enable, const QString &host, const QString &port, const QString &password, const QString &requiresAuth, const QString &username, seb::settings::browser::proxy::ProxyProtocol protocol) {
            if (!boolean(proxy.value(enable))) return;
            seb::settings::browser::proxy::ProxyConfiguration item;
            item.Protocol = protocol;
            item.Host = string(proxy.value(host)); item.Port = integer(proxy.value(port)); item.Password = string(proxy.value(password));
            item.RequiresAuthentication = boolean(proxy.value(requiresAuth)); item.Username = string(proxy.value(username));
            browser.Proxy.Proxies.push_back(item);
        };
        appendProxy(keys::Browser::Proxy::Ftp::Enable, keys::Browser::Proxy::Ftp::Host, keys::Browser::Proxy::Ftp::Port, keys::Browser::Proxy::Ftp::Password, keys::Browser::Proxy::Ftp::RequiresAuthentication, keys::Browser::Proxy::Ftp::Username, seb::settings::browser::proxy::ProxyProtocol::Ftp);
        appendProxy(keys::Browser::Proxy::Http::Enable, keys::Browser::Proxy::Http::Host, keys::Browser::Proxy::Http::Port, keys::Browser::Proxy::Http::Password, keys::Browser::Proxy::Http::RequiresAuthentication, keys::Browser::Proxy::Http::Username, seb::settings::browser::proxy::ProxyProtocol::Http);
        appendProxy(keys::Browser::Proxy::Https::Enable, keys::Browser::Proxy::Https::Host, keys::Browser::Proxy::Https::Port, keys::Browser::Proxy::Https::Password, keys::Browser::Proxy::Https::RequiresAuthentication, keys::Browser::Proxy::Https::Username, seb::settings::browser::proxy::ProxyProtocol::Https);
        appendProxy(keys::Browser::Proxy::Socks::Enable, keys::Browser::Proxy::Socks::Host, keys::Browser::Proxy::Socks::Port, keys::Browser::Proxy::Socks::Password, keys::Browser::Proxy::Socks::RequiresAuthentication, keys::Browser::Proxy::Socks::Username, seb::settings::browser::proxy::ProxyProtocol::Socks);
    }
    mapWindow(browser.MainWindow, key, value, false);
    mapWindow(browser.AdditionalWindow, key, value, true);
}

void BrowserDataMapper::mapGlobal(const QVariantMap &data, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    if (data.contains(keys::Browser::PopupPolicy)) {
        const int policy = integer(data.value(keys::Browser::PopupPolicy), 0);
        const bool foreign = boolean(data.value(keys::Browser::PopupBlockForeignHost));
        switch (policy) {
        case 2: settings.Browser.PopupPolicy = foreign ? seb::settings::browser::PopupPolicy::AllowSameHost : seb::settings::browser::PopupPolicy::Allow; break;
        case 1: settings.Browser.PopupPolicy = foreign ? seb::settings::browser::PopupPolicy::AllowSameHostAndWindow : seb::settings::browser::PopupPolicy::AllowSameWindow; break;
        case 0: settings.Browser.PopupPolicy = seb::settings::browser::PopupPolicy::Block; break;
        default: break;
        }
    }
    const bool processMain = boolean(data.value(keys::Browser::Filter::EnableMainRequestFilter));
    const bool processContent = boolean(data.value(keys::Browser::Filter::EnableContentRequestFilter));
    settings.Browser.Filter.ProcessMainRequests = processMain;
    settings.Browser.Filter.ProcessContentRequests = processMain && processContent;
    const bool mobile = integer(data.value(keys::Browser::UserAgentModeMobile), 0) != 0;
    const bool desktop = integer(data.value(keys::Browser::UserAgentModeDesktop), 0) != 0;
    if (mobile && settings.UserInterface.Mode == seb::settings::userinterface::UserInterfaceMode::Mobile) {
        settings.Browser.UseCustomUserAgent = true;
        settings.Browser.CustomUserAgent = data.value(keys::Browser::CustomUserAgentMobile).toString();
    } else if (desktop) {
        settings.Browser.UseCustomUserAgent = true;
        settings.Browser.CustomUserAgent = data.value(keys::Browser::CustomUserAgentDesktop).toString();
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping
