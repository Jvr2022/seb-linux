#include "browser_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {

namespace {
using namespace helpers;

void mapWindow(seb::settings::browser::WindowSettings &window, const QString &key, const QVariant &value, bool additional)
{
    if (key == (!additional ? keys::Browser::MainWindow::AllowAddressBar : keys::Browser::AdditionalWindow::AllowAddressBar)) window.AllowAddressBar = boolean(value, window.AllowAddressBar);
    else if (key == (!additional ? keys::Browser::MainWindow::AllowNavigation : keys::Browser::AdditionalWindow::AllowNavigation)) { window.AllowBackwardNavigation = boolean(value, window.AllowBackwardNavigation); window.AllowForwardNavigation = window.AllowBackwardNavigation; }
    else if (key == (!additional ? keys::Browser::MainWindow::AllowReload : keys::Browser::AdditionalWindow::AllowReload)) window.AllowReloading = boolean(value, window.AllowReloading);
    else if (!additional && key == keys::Browser::MainWindow::WindowMode && isInteger(value)) window.FullScreenMode = integer(value) == 1;
    else if (key == (!additional ? keys::Browser::MainWindow::ShowReloadWarning : keys::Browser::AdditionalWindow::ShowReloadWarning)) window.ShowReloadWarning = boolean(value, window.ShowReloadWarning);
    else if (key == (!additional ? keys::Browser::MainWindow::WindowHeight : keys::Browser::AdditionalWindow::WindowHeight)) dimension(value, window.AbsoluteHeight, window.RelativeHeight);
    else if (key == (!additional ? keys::Browser::MainWindow::WindowWidth : keys::Browser::AdditionalWindow::WindowWidth)) dimension(value, window.AbsoluteWidth, window.RelativeWidth);
    else if (key == (!additional ? keys::Browser::MainWindow::WindowPosition : keys::Browser::AdditionalWindow::WindowPosition) && isInteger(value)) {
        switch (integer(value)) {
        case 0: window.Position = seb::settings::browser::WindowPosition::Left; break;
        case 1: window.Position = seb::settings::browser::WindowPosition::Center; break;
        case 2: window.Position = seb::settings::browser::WindowPosition::Right; break;
        default: break;
        }
    }
    else if (key == (!additional ? keys::Browser::MainWindow::UrlPolicy : keys::Browser::AdditionalWindow::UrlPolicy) && isInteger(value)) {
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
    if (!isList(value)) return;
    for (const QVariant &entry : value.toList()) {
        if (!isMap(entry)) continue;
        const QVariantMap raw = entry.toMap();
        seb::settings::browser::filter::FilterRuleSettings rule;
        rule.Active = isBoolean(raw.value(QStringLiteral("active"))) && boolean(raw.value(QStringLiteral("active")));
        rule.Expression = string(raw.value(QStringLiteral("expression")));
        if (isInteger(raw.value(QStringLiteral("action")))) {
            rule.Result = integer(raw.value(QStringLiteral("action"))) == 1 ? seb::settings::browser::filter::FilterResult::Allow : seb::settings::browser::filter::FilterResult::Block;
        }
        if (isBoolean(raw.value(QStringLiteral("regex")))) {
            rule.Type = boolean(raw.value(QStringLiteral("regex"))) ? seb::settings::browser::filter::FilterRuleType::Regex : seb::settings::browser::filter::FilterRuleType::Simplified;
        }
        if (rule.Active) filter.Rules.push_back(rule);
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
    else if (key == keys::Browser::DownloadPdfFiles) browser.AllowPdfReader = !boolean(value, !browser.AllowPdfReader);
    else if (key == keys::Browser::AllowDeveloperConsole) { browser.MainWindow.AllowDeveloperConsole = boolean(value, browser.MainWindow.AllowDeveloperConsole); browser.AdditionalWindow.AllowDeveloperConsole = browser.MainWindow.AllowDeveloperConsole; }
    else if (key == keys::Browser::AllowPdfReaderToolbar) browser.AllowPdfReaderToolbar = boolean(value, browser.AllowPdfReaderToolbar);
    else if (key == keys::Browser::AllowPrint) browser.AllowPrint = boolean(value, browser.AllowPrint);
    else if (key == keys::Browser::AllowSpellChecking) browser.AllowSpellChecking = boolean(value, browser.AllowSpellChecking);
    else if (key == keys::Browser::AllowUploads) browser.AllowUploads = boolean(value, browser.AllowUploads);
    else if (key == keys::Browser::ExamKeySalt && isByteArray(value)) browser.BrowserExamKeySalt = value.toByteArray();
    else if (key == keys::Browser::DeleteCacheOnShutdown) browser.DeleteCacheOnShutdown = boolean(value, browser.DeleteCacheOnShutdown);
    else if (key == keys::Browser::DeleteCookiesOnShutdown) browser.DeleteCookiesOnShutdown = boolean(value, browser.DeleteCookiesOnShutdown);
    else if (key == keys::Browser::DeleteCookiesOnStartup) browser.DeleteCookiesOnStartup = boolean(value, browser.DeleteCookiesOnStartup);
    else if (key == keys::Browser::DownloadDirectory) browser.DownAndUploadDirectory = string(value, browser.DownAndUploadDirectory);
    else if (key == keys::Browser::EnableBrowser) browser.EnableBrowser = boolean(value, browser.EnableBrowser);
    else if (key == keys::Browser::Filter::FilterRules) mapFilter(browser.Filter, value);
    else if (key == keys::Browser::HomeButtonMessage) browser.HomeNavigationMessage = string(value, browser.HomeNavigationMessage);
    else if (key == keys::Browser::HomeButtonRequiresPassword) browser.HomeNavigationRequiresPassword = boolean(value, browser.HomeNavigationRequiresPassword);
    else if (key == keys::Browser::HomeButtonUrl) browser.HomeUrl = string(value, browser.HomeUrl);
    else if (key == keys::Browser::QuitUrl) browser.QuitUrl = string(value, browser.QuitUrl);
    else if (key == keys::Browser::QuitUrlConfirmation) browser.ConfirmQuitUrl = boolean(value, browser.ConfirmQuitUrl);
    else if (key == keys::Browser::ResetOnQuitUrl) browser.ResetOnQuitUrl = boolean(value, browser.ResetOnQuitUrl);
    else if (key == keys::Browser::SendCustomHeaders) { browser.SendBrowserExamKey = boolean(value, browser.SendBrowserExamKey); browser.SendConfigurationKey = browser.SendBrowserExamKey; }
    else if (key == keys::Browser::ShowFileSystemElementPath) browser.ShowFileSystemElementPath = boolean(value, browser.ShowFileSystemElementPath);
    else if (key == keys::Browser::StartUrl) browser.StartUrl = string(value, browser.StartUrl);
    else if (key == keys::Browser::UseStartUrlQuery) browser.UseQueryParameter = boolean(value, browser.UseQueryParameter);
    else if (key == keys::Browser::UserAgentSuffix) browser.UserAgentSuffix = string(value, browser.UserAgentSuffix);
    else if (key == keys::Browser::HomeButtonUseStartUrl) browser.UseStartUrlAsHomeUrl = boolean(value, browser.UseStartUrlAsHomeUrl);
    else if (key == keys::Browser::UseTemporaryDownUploadDirectory) browser.UseTemporaryDownAndUploadDirectory = boolean(value, browser.UseTemporaryDownAndUploadDirectory);
    else if (key == keys::Browser::ShowReloadButton) { browser.MainWindow.ShowReloadButton = boolean(value, browser.MainWindow.ShowReloadButton); browser.AdditionalWindow.ShowReloadButton = browser.MainWindow.ShowReloadButton; }
    else if (key == keys::Browser::ShowToolbar) { browser.MainWindow.ShowToolbar = boolean(value, browser.MainWindow.ShowToolbar); browser.AdditionalWindow.ShowToolbar = browser.MainWindow.ShowToolbar; }
    else if (key == keys::Browser::Filter::EnableContentRequestFilter) browser.Filter.ProcessContentRequests = boolean(value, browser.Filter.ProcessContentRequests);
    else if (key == keys::Browser::Filter::EnableMainRequestFilter) browser.Filter.ProcessMainRequests = boolean(value, browser.Filter.ProcessMainRequests);
    else if (key == keys::Browser::Proxy::Policy && isInteger(value)) browser.Proxy.Policy = integer(value) == 1 ? seb::settings::browser::proxy::ProxyPolicy::Custom : seb::settings::browser::proxy::ProxyPolicy::System;
    else if (key == keys::Browser::Proxy::Settings && isMap(value)) {
        const QVariantMap proxy = value.toMap();
        browser.Proxy.AutoConfigure = boolean(proxy.value(keys::Browser::Proxy::AutoConfigure));
        browser.Proxy.AutoDetect = boolean(proxy.value(keys::Browser::Proxy::AutoDetect));
        browser.Proxy.AutoConfigureUrl = string(proxy.value(keys::Browser::Proxy::AutoConfigureUrl), browser.Proxy.AutoConfigureUrl);
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
    if (data.contains(keys::Browser::PopupPolicy) && isInteger(data.value(keys::Browser::PopupPolicy))) {
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
    if (settings.UserInterface.Mode == seb::settings::userinterface::UserInterfaceMode::Desktop && desktop) {
        settings.Browser.UseCustomUserAgent = true;
        if (isString(data.value(keys::Browser::CustomUserAgentDesktop))) {
            settings.Browser.CustomUserAgent = data.value(keys::Browser::CustomUserAgentDesktop).toString();
        }
    } else if (settings.UserInterface.Mode == seb::settings::userinterface::UserInterfaceMode::Mobile && mobile) {
        settings.Browser.UseCustomUserAgent = true;
        if (isString(data.value(keys::Browser::CustomUserAgentMobile))) {
            settings.Browser.CustomUserAgent = data.value(keys::Browser::CustomUserAgentMobile).toString();
        }
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping
