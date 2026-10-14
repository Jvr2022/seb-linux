#pragma once

#include <QString>

namespace seb::configuration::configuration_data::keys {

inline constexpr int WINDOWS = 1;

namespace Applications {
inline const QString Active = QStringLiteral("active");
inline const QString AllowCustomPath = QStringLiteral("allowUserToChooseApp");
inline const QString AllowRunning = QStringLiteral("runInBackground");
inline const QString Argument = QStringLiteral("argument");
inline const QString Arguments = QStringLiteral("arguments");
inline const QString AutoStart = QStringLiteral("autostart");
inline const QString AutoTerminate = QStringLiteral("strongKill");
inline const QString Blacklist = QStringLiteral("prohibitedProcesses");
inline const QString Description = QStringLiteral("description");
inline const QString DisplayName = QStringLiteral("title");
inline const QString ExecutableName = QStringLiteral("executable");
inline const QString ExecutablePath = QStringLiteral("path");
inline const QString OperatingSystem = QStringLiteral("os");
inline const QString OriginalName = QStringLiteral("originalName");
inline const QString ShowInShell = QStringLiteral("iconInTaskbar");
inline const QString Signature = QStringLiteral("signature");
inline const QString Whitelist = QStringLiteral("permittedProcesses");
}
namespace Audio {
inline const QString InitialVolume = QStringLiteral("audioVolumeLevel");
inline const QString InitializeVolume = QStringLiteral("audioSetVolumeLevel");
inline const QString MuteAudio = QStringLiteral("audioMute");
}
namespace Browser {
inline const QString AllowConfigurationDownloads = QStringLiteral("downloadAndOpenSebConfig");
inline const QString AllowCustomDownUploadLocation = QStringLiteral("allowCustomDownUploadLocation");
inline const QString AllowDeveloperConsole = QStringLiteral("allowDeveloperConsole");
inline const QString AllowDownloads = QStringLiteral("allowDownloads");
inline const QString AllowFind = QStringLiteral("allowFind");
inline const QString AllowPageZoom = QStringLiteral("enableZoomPage");
inline const QString AllowPdfReader = QStringLiteral("enablePDFReader");
inline const QString DownloadPdfFiles = QStringLiteral("downloadPDFFiles");
inline const QString AllowPdfReaderToolbar = QStringLiteral("allowPDFReaderToolbar");
inline const QString AllowPrint = QStringLiteral("allowPrint");
inline const QString AllowSpellChecking = QStringLiteral("allowSpellCheck");
inline const QString AllowUploads = QStringLiteral("allowUploads");
inline const QString BrowserExamKeySalt = QStringLiteral("examKeySalt");
inline const QString CustomBrowserExamKey = QStringLiteral("browserExamKey");
inline const QString CustomUserAgent = QStringLiteral("browserUserAgentWinDesktopModeCustom");
inline const QString CustomUserAgentDesktop = QStringLiteral("browserUserAgentWinDesktopModeCustom");
inline const QString DeleteCacheOnShutdown = QStringLiteral("removeBrowserProfile");
inline const QString DeleteCookiesOnShutdown = QStringLiteral("examSessionClearCookiesOnEnd");
inline const QString DeleteCookiesOnStartup = QStringLiteral("examSessionClearCookiesOnStart");
inline const QString DownAndUploadDirectory = QStringLiteral("downloadDirectoryWin");
inline const QString DownloadDirectory = QStringLiteral("downloadDirectoryWin");
inline const QString EnableBrowser = QStringLiteral("enableSebBrowser");
inline const QString FilterRules = QStringLiteral("URLFilterRules");
inline const QString HomeNavigationMessage = QStringLiteral("restartExamText");
inline const QString HomeNavigationRequiresPassword = QStringLiteral("restartExamPasswordProtected");
inline const QString HomePasswordHash = QStringLiteral("hashedRestartExamPassword");
inline const QString HomeUrl = QStringLiteral("restartExamURL");
inline const QString HomeButtonMessage = QStringLiteral("restartExamText");
inline const QString HomeButtonRequiresPassword = QStringLiteral("restartExamPasswordProtected");
inline const QString HomeButtonUrl = QStringLiteral("restartExamURL");
inline const QString HomeButtonUseStartUrl = QStringLiteral("restartExamUseStartURL");
inline const QString NewBrowserWindowByLinkBlockForeign = QStringLiteral("newBrowserWindowByLinkBlockForeign");
inline const QString NewBrowserWindowByLinkPolicy = QStringLiteral("newBrowserWindowByLinkPolicy");
inline const QString PopupPolicy = QStringLiteral("newBrowserWindowByLinkPolicy");
inline const QString PopupBlockForeignHost = QStringLiteral("newBrowserWindowByLinkBlockForeign");
inline const QString ProxyPolicy = QStringLiteral("proxySettingsPolicy");
inline const QString ProxySettings = QStringLiteral("proxies");
inline const QString ShowReloadButton = QStringLiteral("showReloadButton");
inline const QString ShowToolbar = QStringLiteral("enableBrowserWindowToolbar");
inline const QString QuitUrl = QStringLiteral("quitURL");
inline const QString QuitUrlConfirm = QStringLiteral("quitURLConfirm");
inline const QString QuitUrlRestart = QStringLiteral("quitURLRestart");
inline const QString ResetOnQuitUrl = QStringLiteral("quitURLRestart");
inline const QString SendBrowserExamKey = QStringLiteral("sendBrowserExamKey");
inline const QString SendCustomHeaders = QStringLiteral("sendBrowserExamKey");
inline const QString SendConfigurationKey = QStringLiteral("sendConfigurationKey");
inline const QString ShowFileSystemElementPath = QStringLiteral("browserShowFileSystemElementPath");
inline const QString StartUrl = QStringLiteral("startURL");
inline const QString StartUrlQuery = QStringLiteral("startURLAppendQueryParameter");
inline const QString UserAgentSuffix = QStringLiteral("browserUserAgent");
inline const QString UserAgentMode = QStringLiteral("browserUserAgentWinDesktopMode");
inline const QString UserAgentModeMobile = QStringLiteral("browserUserAgentWinTouchMode");
inline const QString CustomUserAgentMobile = QStringLiteral("browserUserAgentWinTouchModeCustom");
inline const QString UseCustomUserAgent = QStringLiteral("browserUserAgentWinDesktopModeCustom");
inline const QString UseQueryParameter = QStringLiteral("startURLAppendQueryParameter");
inline const QString UseStartUrlAsHomeUrl = QStringLiteral("restartExamUseStartURL");
inline const QString UseTemporaryDownAndUploadDirectory = QStringLiteral("useTemporaryDownUploadDirectory");
namespace AdditionalWindow {
inline const QString AllowAddressBar = QStringLiteral("newBrowserWindowAllowAddressBar");
inline const QString AllowNavigation = QStringLiteral("newBrowserWindowNavigation");
inline const QString AllowReload = QStringLiteral("newBrowserWindowAllowReload");
inline const QString ShowReloadWarning = QStringLiteral("newBrowserWindowShowReloadWarning");
inline const QString UrlPolicy = QStringLiteral("newBrowserWindowShowURL");
inline const QString WindowHeight = QStringLiteral("newBrowserWindowByLinkHeight");
inline const QString WindowPosition = QStringLiteral("newBrowserWindowByLinkPositioning");
inline const QString WindowWidth = QStringLiteral("newBrowserWindowByLinkWidth");
}
namespace MainWindow {
inline const QString AllowAddressBar = QStringLiteral("browserWindowAllowAddressBar");
inline const QString AllowDeveloperConsole = QStringLiteral("browserWindowAllowDeveloperConsole");
inline const QString AllowMinimize = QStringLiteral("browserWindowAllowMinimize");
inline const QString AllowNavigation = QStringLiteral("allowBrowsingBackForward");
inline const QString AllowReload = QStringLiteral("browserWindowAllowReload");
inline const QString AlwaysOnTop = QStringLiteral("browserWindowAlwaysOnTop");
inline const QString FullScreenMode = QStringLiteral("browserViewMode");
inline const QString ShowHomeButton = QStringLiteral("browserWindowShowHomeButton");
inline const QString ShowMenuBar = QStringLiteral("browserWindowShowMenuBar");
inline const QString ShowReloadButton = QStringLiteral("browserWindowShowReloadButton");
inline const QString ShowReloadWarning = QStringLiteral("showReloadWarning");
inline const QString ShowToolbar = QStringLiteral("browserWindowShowToolbar");
inline const QString ShowTitleBar = QStringLiteral("browserWindowShowTitleBar");
inline const QString UrlPolicy = QStringLiteral("browserWindowShowURL");
inline const QString WindowHeight = QStringLiteral("mainBrowserWindowHeight");
inline const QString WindowPosition = QStringLiteral("mainBrowserWindowPositioning");
inline const QString WindowWidth = QStringLiteral("mainBrowserWindowWidth");
inline const QString WindowMode = QStringLiteral("browserViewMode");
}
namespace Filter {
inline const QString ProcessContentRequests = QStringLiteral("URLFilterEnableContentFilter");
inline const QString ProcessMainRequests = QStringLiteral("URLFilterEnable");
inline const QString EnableContentRequestFilter = QStringLiteral("URLFilterEnableContentFilter");
inline const QString EnableMainRequestFilter = QStringLiteral("URLFilterEnable");
inline const QString RuleAction = QStringLiteral("action");
inline const QString RuleIsActive = QStringLiteral("active");
inline const QString RuleExpression = QStringLiteral("expression");
inline const QString RuleExpressionIsRegex = QStringLiteral("regex");
}
namespace Proxy {
inline const QString Policy = QStringLiteral("proxySettingsPolicy");
inline const QString Settings = QStringLiteral("proxies");
inline const QString AutoConfigure = QStringLiteral("AutoConfigurationEnabled");
inline const QString AutoConfigureUrl = QStringLiteral("AutoConfigurationURL");
inline const QString AutoDetect = QStringLiteral("AutoDiscoveryEnabled");
inline const QString BypassList = QStringLiteral("ExceptionsList");
namespace Ftp { inline const QString Enable = QStringLiteral("FTPEnable"); inline const QString Host = QStringLiteral("FTPProxy"); inline const QString Port = QStringLiteral("FTPPort"); inline const QString Password = QStringLiteral("FTPPassword"); inline const QString RequiresAuthentication = QStringLiteral("FTPRequiresPassword"); inline const QString Username = QStringLiteral("FTPUsername"); }
namespace Http { inline const QString Enable = QStringLiteral("HTTPEnable"); inline const QString Host = QStringLiteral("HTTPProxy"); inline const QString Port = QStringLiteral("HTTPPort"); inline const QString Password = QStringLiteral("HTTPPassword"); inline const QString RequiresAuthentication = QStringLiteral("HTTPRequiresPassword"); inline const QString Username = QStringLiteral("HTTPUsername"); }
namespace Https { inline const QString Enable = QStringLiteral("HTTPSEnable"); inline const QString Host = QStringLiteral("HTTPSProxy"); inline const QString Port = QStringLiteral("HTTPSPort"); inline const QString Password = QStringLiteral("HTTPSPassword"); inline const QString RequiresAuthentication = QStringLiteral("HTTPSRequiresPassword"); inline const QString Username = QStringLiteral("HTTPSUsername"); }
namespace Socks { inline const QString Enable = QStringLiteral("SOCKSEnable"); inline const QString Host = QStringLiteral("SOCKSProxy"); inline const QString Port = QStringLiteral("SOCKSPort"); inline const QString Password = QStringLiteral("SOCKSPassword"); inline const QString RequiresAuthentication = QStringLiteral("SOCKSRequiresPassword"); inline const QString Username = QStringLiteral("SOCKSUsername"); }
}
}
namespace ConfigurationFile {
inline const QString ConfigurationPurpose = QStringLiteral("sebConfigPurpose");
inline const QString KeepClientConfigEncryption = QStringLiteral("clientConfigKeepEncryption");
inline const QString SessionMode = QStringLiteral("sebMode");
}
namespace Display {
inline const QString AllowedDisplays = QStringLiteral("allowedDisplaysMaxNumber");
inline const QString AlwaysOn = QStringLiteral("displayAlwaysOn");
inline const QString IgnoreError = QStringLiteral("allowedDisplaysIgnoreFailure");
inline const QString InternalDisplayOnly = QStringLiteral("allowedDisplayBuiltinEnforce");
}
namespace General { inline const QString LogLevel = QStringLiteral("logLevel"); inline const QString OriginatorVersion = QStringLiteral("originatorVersion"); }
namespace Keyboard {
inline const QString AllowAltEsc = QStringLiteral("enableAltEsc"); inline const QString AllowAltF4 = QStringLiteral("enableAltF4");
inline const QString AllowAltTab = QStringLiteral("enableAltTab"); inline const QString AllowCtrlC = QStringLiteral("enableCtrlC");
inline const QString AllowCtrlV = QStringLiteral("enableCtrlV"); inline const QString AllowCtrlX = QStringLiteral("enableCtrlX");
inline const QString AllowCtrlEsc = QStringLiteral("enableCtrlEsc"); inline const QString AllowEsc = QStringLiteral("enableEsc");
inline const QString AllowInjected = QStringLiteral("enableInjected"); inline const QString AllowPrintScreen = QStringLiteral("enablePrintScreen");
inline const QString AllowSystemKey = QStringLiteral("enableStartMenu"); inline const QString EnableKeyCheck = QStringLiteral("enableKeyMonitoring");
inline const QString EnableLayoutCheck = QStringLiteral("enableKeyboardLayoutCheck");
inline const QString EnableAltEsc = QStringLiteral("enableAltEsc"); inline const QString EnableAltF4 = QStringLiteral("enableAltF4"); inline const QString EnableAltTab = QStringLiteral("enableAltTab");
inline const QString EnableCtrlEsc = QStringLiteral("enableCtrlEsc"); inline const QString EnableEsc = QStringLiteral("enableEsc");
inline const QString EnableF1 = QStringLiteral("enableF1"); inline const QString EnableF2 = QStringLiteral("enableF2"); inline const QString EnableF3 = QStringLiteral("enableF3"); inline const QString EnableF4 = QStringLiteral("enableF4");
inline const QString EnableF5 = QStringLiteral("enableF5"); inline const QString EnableF6 = QStringLiteral("enableF6"); inline const QString EnableF7 = QStringLiteral("enableF7"); inline const QString EnableF8 = QStringLiteral("enableF8");
inline const QString EnableF9 = QStringLiteral("enableF9"); inline const QString EnableF10 = QStringLiteral("enableF10"); inline const QString EnableF11 = QStringLiteral("enableF11"); inline const QString EnableF12 = QStringLiteral("enableF12");
inline const QString EnableInjected = QStringLiteral("enableInjected"); inline const QString EnablePrintScreen = QStringLiteral("enablePrintScreen"); inline const QString EnableSystemKey = QStringLiteral("enableStartMenu");
}
namespace Mouse {
inline const QString AllowMiddleButton = QStringLiteral("enableMiddleMouse"); inline const QString AllowRightButton = QStringLiteral("enableRightMouse");
inline const QString EnableMovementCheck = QStringLiteral("enableMouseMonitoring");
inline const QString EnableMiddleMouseButton = QStringLiteral("enableMiddleMouse"); inline const QString EnableRightMouseButton = QStringLiteral("enableRightMouse");
}
namespace Network {
inline const QString AllowWlan = QStringLiteral("allowWlan");
namespace Certificates {
inline const QString CertificateData = QStringLiteral("certificateData");
inline const QString CertificateType = QStringLiteral("type");
inline const QString EmbeddedCertificates = QStringLiteral("embeddedCertificates");
}
}
namespace Proctoring {
inline const QString ShowDisclaimer = QStringLiteral("showProctoringDisclaimer");
inline const QString ShowTaskbarNotification = QStringLiteral("showProctoringViewButton");
inline const QString ClientId = QStringLiteral("screenProctoringClientId"); inline const QString ClientSecret = QStringLiteral("screenProctoringClientSecret");
inline const QString Enabled = QStringLiteral("enableScreenProctoring"); inline const QString ServiceUrl = QStringLiteral("screenProctoringServiceURL");
inline const QString ScreenCacheSize = QStringLiteral("screenProctoringCacheSize"); inline const QString ScreenEnabled = QStringLiteral("enableScreenProctoring"); inline const QString ScreenGroupId = QStringLiteral("screenProctoringGroupId");
inline const QString ScreenImageDownscaling = QStringLiteral("screenProctoringImageDownscale"); inline const QString ScreenImageFormat = QStringLiteral("screenProctoringImageFormat");
inline const QString ScreenImageQuantization = QStringLiteral("screenProctoringImageQuantization"); inline const QString ScreenIntervalMaximum = QStringLiteral("screenProctoringScreenshotMaxInterval");
inline const QString ScreenIntervalMinimum = QStringLiteral("screenProctoringScreenshotMinInterval"); inline const QString ScreenShowDisclaimer = QStringLiteral("showProctoringDisclaimer");
inline const QString ScreenShowTaskbarNotification = QStringLiteral("showProctoringViewButton");
inline const QString CaptureApplicationData = QStringLiteral("screenProctoringMetadataActiveAppEnabled"); inline const QString CaptureBrowserData = QStringLiteral("screenProctoringMetadataURLEnabled");
inline const QString CaptureWindowTitle = QStringLiteral("screenProctoringMetadataWindowTitleEnabled");
namespace ScreenProctoring {
inline const QString CacheSize = QStringLiteral("screenProctoringCacheSize"); inline const QString ClientId = QStringLiteral("screenProctoringClientId"); inline const QString ClientSecret = QStringLiteral("screenProctoringClientSecret");
inline const QString Enabled = QStringLiteral("enableScreenProctoring"); inline const QString GroupId = QStringLiteral("screenProctoringGroupId"); inline const QString ImageDownscaling = QStringLiteral("screenProctoringImageDownscale"); inline const QString ImageFormat = QStringLiteral("screenProctoringImageFormat"); inline const QString ImageQuantization = QStringLiteral("screenProctoringImageQuantization"); inline const QString IntervalMaximum = QStringLiteral("screenProctoringScreenshotMaxInterval"); inline const QString IntervalMinimum = QStringLiteral("screenProctoringScreenshotMinInterval"); inline const QString ServiceUrl = QStringLiteral("screenProctoringServiceURL");
namespace MetaData { inline const QString CaptureApplicationData = QStringLiteral("screenProctoringMetadataActiveAppEnabled"); inline const QString CaptureBrowserData = QStringLiteral("screenProctoringMetadataURLEnabled"); inline const QString CaptureWindowTitle = QStringLiteral("screenProctoringMetadataWindowTitleEnabled"); }
}
}
namespace Security {
inline const QString AdminPasswordHash = QStringLiteral("hashedAdminPassword"); inline const QString AllowApplicationLogAccess = QStringLiteral("allowApplicationLog");
inline const QString AllowReconfiguration = QStringLiteral("examSessionReconfigureAllow"); inline const QString AllowStickyKeys = QStringLiteral("allowStickyKeys");
inline const QString AllowTermination = QStringLiteral("allowQuit"); inline const QString AllowVirtualMachine = QStringLiteral("allowVirtualMachine");
inline const QString AllowWindowCapture = QStringLiteral("allowScreenSharing"); inline const QString ClipboardPolicy = QStringLiteral("clipboardPolicy");
inline const QString DisableSessionChangeLockScreen = QStringLiteral("disableSessionChangeLockScreen"); inline const QString KioskModeCreateNewDesktop = QStringLiteral("createNewDesktop");
inline const QString KioskModeDisableExplorerShell = QStringLiteral("killExplorerShell"); inline const QString QuitPasswordHash = QStringLiteral("hashedQuitPassword");
inline const QString ReconfigurationUrl = QStringLiteral("examSessionReconfigureConfigURL"); inline const QString VerifyCursorConfiguration = QStringLiteral("enableCursorVerification");
inline const QString VerifySessionIntegrity = QStringLiteral("enableSessionVerification"); inline const QString VersionRestrictions = QStringLiteral("sebAllowedVersions");
}
namespace Server {
inline const QString ApiEndpoint = QStringLiteral("apiDiscovery"); inline const QString ExamId = QStringLiteral("exam");
inline const QString ClientName = QStringLiteral("clientName"); inline const QString ClientSecretKey = QStringLiteral("clientSecret"); inline const QString Institution = QStringLiteral("institution");
inline const QString ConfigurationData = QStringLiteral("sebServerConfiguration"); inline const QString FallbackPasswordHash = QStringLiteral("sebServerFallbackPasswordHash"); inline const QString ShowRaiseHand = QStringLiteral("raiseHandButtonShow");
inline const QString ForceRaiseHandMessage = QStringLiteral("raiseHandButtonAlwaysPromptMessage");
inline const QString PerformFallback = QStringLiteral("sebServerFallback"); inline const QString PingInterval = QStringLiteral("pingInterval");
inline const QString RequestAttempts = QStringLiteral("sebServerFallbackAttempts"); inline const QString RequestAttemptInterval = QStringLiteral("sebServerFallbackAttemptInterval");
inline const QString RequestTimeout = QStringLiteral("sebServerFallbackTimeout");
inline const QString Enabled = QStringLiteral("sebServerConfiguration"); inline const QString Url = QStringLiteral("sebServerURL"); inline const QString Port = QStringLiteral("sebServerPort");
inline const QString Configuration = QStringLiteral("sebServerConfiguration"); inline const QString ServerUrl = QStringLiteral("sebServerURL");
inline const QString ClientId = QStringLiteral("clientName"); inline const QString ClientSecret = QStringLiteral("clientSecret");
}
namespace Service {
inline const QString EnableChromeNotifications = QStringLiteral("enableChromeNotifications");
inline const QString EnableEaseOfAccessOptions = QStringLiteral("insideSebEnableEaseOfAccess");
inline const QString EnableFindPrinter = QStringLiteral("enableFindPrinter");
inline const QString EnableNetworkOptions = QStringLiteral("insideSebEnableNetworkConnectionSelector");
inline const QString EnablePasswordChange = QStringLiteral("insideSebEnableChangeAPassword");
inline const QString EnablePowerOptions = QStringLiteral("insideSebEnableShutDown");
inline const QString EnableRemoteConnections = QStringLiteral("allowScreenSharing");
inline const QString EnableSignout = QStringLiteral("insideSebEnableLogOff");
inline const QString EnableTaskManager = QStringLiteral("insideSebEnableStartTaskManager");
inline const QString DisableUserLock = QStringLiteral("insideSebEnableLockThisComputer"); inline const QString DisableUserSwitch = QStringLiteral("insideSebEnableSwitchUser");
inline const QString EnableUserLock = QStringLiteral("insideSebEnableLockThisComputer"); inline const QString EnableUserSwitch = QStringLiteral("insideSebEnableSwitchUser");
inline const QString EnableVmwareOverlay = QStringLiteral("insideSebEnableVmWareClientShade");
inline const QString EnableWindowsUpdate = QStringLiteral("enableWindowsUpdate");
inline const QString IgnoreService = QStringLiteral("sebServiceIgnore"); inline const QString Policy = QStringLiteral("sebServicePolicy");
inline const QString SetVmwareConfiguration = QStringLiteral("setVmwareConfiguration");
}
namespace System { inline const QString AlwaysOn = QStringLiteral("systemAlwaysOn"); inline const QString AlwaysOnTop = QStringLiteral("systemAlwaysOn"); inline const QString ShowApplicationChooser = QStringLiteral("showApplicationChooser"); }
namespace UserInterface {
inline const QString EnableActionCenter = QStringLiteral("showSideMenu"); inline const QString EnableLockScreen = QStringLiteral("enableLockScreen"); inline const QString LockScreenBackgroundColor = QStringLiteral("lockScreenBackgroundColor");
inline const QString EnableTaskbar = QStringLiteral("showTaskBar"); inline const QString ShowApplicationInfo = QStringLiteral("showQRVerifyButton"); inline const QString ShowVerificator = QStringLiteral("showQRVerifyButton");
inline const QString ShowApplicationLog = QStringLiteral("showApplicationLogButton"); inline const QString ShowAudioControl = QStringLiteral("audioControlEnabled");
inline const QString ShowClock = QStringLiteral("showTime"); inline const QString ShowKeyboardLayout = QStringLiteral("showInputLanguage"); inline const QString ShowNetwork = QStringLiteral("allowWlan");
inline const QString UserInterfaceMode = QStringLiteral("touchOptimized"); inline const QString ChargeThresholdCritical = QStringLiteral("batteryChargeThresholdCritical"); inline const QString ChargeThresholdLow = QStringLiteral("batteryChargeThresholdLow");
namespace ActionCenter { inline const QString EnableActionCenter = QStringLiteral("showSideMenu"); }
namespace LockScreen { inline const QString BackgroundColor = QStringLiteral("lockScreenBackgroundColor"); }
namespace SystemControls {
namespace Audio { inline const QString Show = QStringLiteral("audioControlEnabled"); }
namespace Clock { inline const QString Show = QStringLiteral("showTime"); }
namespace KeyboardLayout { inline const QString Show = QStringLiteral("showInputLanguage"); }
namespace Network { inline const QString Show = QStringLiteral("allowWlan"); }
namespace PowerSupply { inline const QString ChargeThresholdCritical = QStringLiteral("batteryChargeThresholdCritical"); inline const QString ChargeThresholdLow = QStringLiteral("batteryChargeThresholdLow"); }
}
namespace Taskbar { inline const QString EnableTaskbar = QStringLiteral("showTaskBar"); inline const QString ShowApplicationLog = QStringLiteral("showApplicationLogButton"); inline const QString ShowVerificator = QStringLiteral("showQRVerifyButton"); }
}
namespace Meta { inline const QString RawSebData = QStringLiteral("__rawSebData"); inline const QString SourceText = QStringLiteral("__sourceText"); }
}
