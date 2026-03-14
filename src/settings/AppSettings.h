#pragma once

#include "Applications/ApplicationSettings.h"
#include "Browser/BrowserSettings.h"
#include "ConfigurationMode.h"
#include "Logging/LogLevel.h"
#include "Monitoring/DisplaySettings.h"
#include "Monitoring/KeyboardSettings.h"
#include "Monitoring/MouseSettings.h"
#include "Proctoring/ProctoringSettings.h"
#include "Security/SecuritySettings.h"
#include "Server/ServerSettings.h"
#include "Service/ServiceSettings.h"
#include "SessionMode.h"
#include "System/SystemSettings.h"
#include "SystemComponents/AudioSettings.h"
#include "SystemComponents/PowerSupplySettings.h"
#include "UserInterface/UserInterfaceSettings.h"

namespace seb::settings {

struct AppSettings
{
    applications::ApplicationSettings Applications;
    systemcomponents::AudioSettings Audio;
    browser::BrowserSettings Browser;
    ConfigurationMode ConfigurationMode = ConfigurationMode::ConfigureClient;
    monitoring::DisplaySettings Display;
    monitoring::KeyboardSettings Keyboard;
    logging::LogLevel LogLevel = logging::LogLevel::Info;
    monitoring::MouseSettings Mouse;
    systemcomponents::PowerSupplySettings PowerSupply;
    proctoring::ProctoringSettings Proctoring;
    security::SecuritySettings Security;
    server::ServerSettings Server;
    service::ServiceSettings Service;
    SessionMode SessionMode = SessionMode::Server;
    system::SystemSettings System;
    userinterface::UserInterfaceSettings UserInterface;
};

}  // namespace seb::settings
