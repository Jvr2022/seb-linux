#include "data_processor.h"

#include "json.h"

#include <QCryptographicHash>

namespace {

bool isLegacyBrowser(const seb::settings::applications::WhitelistApplication &application)
{
    const bool enginePath = application.ExecutablePath.contains(QStringLiteral("xulrunner"));
    const bool firefox = application.ExecutableName.compare(QStringLiteral("firefox.exe"), Qt::CaseInsensitive) == 0;
    const bool xulRunner = application.ExecutableName.compare(QStringLiteral("xulrunner.exe"), Qt::CaseInsensitive) == 0;
    return enginePath && (firefox || xulRunner);
}

}  // namespace

namespace seb::configuration::configuration_data {

void DataProcessor::processDefault(seb::settings::AppSettings &settings) const
{
    allowBrowserToolbarForReloading(settings);
    initializeBrowserHomeFunctionality(settings);
    initializeClipboardSettings(settings);
    initializeProctoringSettings(settings);
    removeLegacyBrowsers(settings);
}

void DataProcessor::process(const QVariantMap &data, seb::settings::AppSettings &settings) const
{
    processDefault(settings);
    calculateConfigurationKey(data, settings);
}

void DataProcessor::allowBrowserToolbarForReloading(seb::settings::AppSettings &settings) const
{
    if (settings.Browser.AdditionalWindow.AllowReloading && settings.Browser.AdditionalWindow.ShowReloadButton) {
        settings.Browser.AdditionalWindow.ShowToolbar = true;
    }
    if (settings.Browser.MainWindow.AllowReloading && settings.Browser.MainWindow.ShowReloadButton) {
        settings.Browser.MainWindow.ShowToolbar = true;
    }
}

void DataProcessor::calculateConfigurationKey(const QVariantMap &data, seb::settings::AppSettings &settings) const
{
    const QByteArray serialized = Json::serialize(data);
    const QByteArray hash = QCryptographicHash::hash(serialized, QCryptographicHash::Sha256);
    settings.Browser.ConfigurationKey = QString::fromLatin1(hash.toHex());
}

void DataProcessor::initializeBrowserHomeFunctionality(seb::settings::AppSettings &settings) const
{
    settings.Browser.MainWindow.ShowHomeButton =
        settings.Browser.UseStartUrlAsHomeUrl || !settings.Browser.HomeUrl.trimmed().isEmpty();
    settings.Browser.HomePasswordHash = settings.Security.QuitPasswordHash;
}

void DataProcessor::initializeClipboardSettings(seb::settings::AppSettings &settings) const
{
    settings.Browser.UseIsolatedClipboard =
        settings.Security.ClipboardPolicy == seb::settings::security::ClipboardPolicy::Isolated;
    const bool clipboardAllowed =
        settings.Security.ClipboardPolicy != seb::settings::security::ClipboardPolicy::Block;
    settings.Keyboard.AllowCtrlC = clipboardAllowed;
    settings.Keyboard.AllowCtrlV = clipboardAllowed;
    settings.Keyboard.AllowCtrlX = clipboardAllowed;
}

void DataProcessor::initializeProctoringSettings(seb::settings::AppSettings &settings) const
{
    settings.Proctoring.Enabled = settings.Proctoring.ScreenProctoring.Enabled;
}

void DataProcessor::removeLegacyBrowsers(seb::settings::AppSettings &settings) const
{
    for (auto application = settings.Applications.Whitelist.begin();
         application != settings.Applications.Whitelist.end();) {
        if (isLegacyBrowser(*application)) {
            application = settings.Applications.Whitelist.erase(application);
        } else {
            ++application;
        }
    }
}

}  // namespace seb::configuration::configuration_data
