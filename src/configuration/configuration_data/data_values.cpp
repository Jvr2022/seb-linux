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

seb::settings::AppSettings DataValues::loadDefaultSettings() const
{
    seb::settings::AppSettings settings;
    settings.Browser.StartUrl = QStringLiteral("https://www.safeexambrowser.org/start");
    settings.Browser.DeleteCacheOnShutdown = true;
    settings.Browser.DeleteCookiesOnShutdown = true;
    settings.Browser.DeleteCookiesOnStartup = true;
    settings.Browser.PopupPolicy = seb::settings::browser::PopupPolicy::Allow;
    settings.Browser.AdditionalWindow.RelativeHeight = 100;
    settings.Browser.AdditionalWindow.RelativeWidth = 50;
    settings.Browser.AdditionalWindow.Position = seb::settings::browser::WindowPosition::Right;
    settings.Browser.AdditionalWindow.UrlPolicy = seb::settings::browser::UrlPolicy::Never;
    settings.Browser.AdditionalWindow.ShowToolbar = false;
    settings.Browser.AdditionalWindow.AllowBackwardNavigation = true;
    settings.Browser.AdditionalWindow.AllowForwardNavigation = true;
    settings.Browser.MainWindow.ShowReloadWarning = true;
    settings.Browser.MainWindow.ShowToolbar = false;
    settings.Browser.MainWindow.UrlPolicy = seb::settings::browser::UrlPolicy::Never;
    settings.Display.AllowedDisplays = 1;
    settings.Display.AlwaysOn = true;
    settings.Display.IgnoreError = false;
    settings.Display.InternalDisplayOnly = false;
    settings.Security.AllowTermination = true;
    settings.Security.AllowWindowCapture = false;
    settings.Security.ClipboardPolicy = seb::settings::security::ClipboardPolicy::Isolated;
    settings.Security.VirtualMachinePolicy = seb::settings::security::VirtualMachinePolicy::Deny;
    settings.Security.KioskMode = seb::settings::security::KioskMode::CreateNewDesktop;
    settings.Security.VerifyCursorConfiguration = true;
    settings.Security.VerifySessionIntegrity = true;
    settings.SessionMode = seb::settings::SessionMode::Normal;
    settings.ConfigurationMode = seb::settings::ConfigurationMode::Exam;
    settings.PowerSupply.ChargeThresholdCritical = 0.1;
    settings.PowerSupply.ChargeThresholdLow = 0.2;
    settings.LogLevel = seb::settings::logging::LogLevel::Debug;
    settings.System.AlwaysOn = true;
    settings.UserInterface.ActionCenter.EnableActionCenter = true;
    settings.UserInterface.Taskbar.ShowVerificator = true;
    settings.UserInterface.LockScreen.BackgroundColor = QStringLiteral("#ff0000");
    settings.Proctoring.ScreenProctoring.CacheSize = 500;
    settings.Proctoring.ScreenProctoring.IntervalMaximum = 5000;
    settings.Proctoring.ScreenProctoring.IntervalMinimum = 1000;
    settings.Proctoring.ScreenProctoring.ImageFormat = seb::settings::proctoring::ImageFormat::Png;
    settings.Proctoring.ScreenProctoring.ImageDownscaling = 1.0;
    settings.Proctoring.ScreenProctoring.Quantization = seb::settings::proctoring::ImageQuantization::Grayscale4bpp;
    settings.Proctoring.ScreenProctoring.MetaData.CaptureApplicationData = true;
    settings.Proctoring.ScreenProctoring.MetaData.CaptureBrowserData = true;
    settings.Proctoring.ScreenProctoring.MetaData.CaptureWindowTitle = true;
    settings.Service.DisableUserLock = true;
    settings.Service.DisableUserSwitch = true;
    settings.Service.Policy = seb::settings::service::ServicePolicy::Mandatory;
    return settings;
}

QString DataValues::getAppDataFilePath() const
{
    return initializeAppConfig().appDataFilePath;
}

}  // namespace seb::configuration::configuration_data
