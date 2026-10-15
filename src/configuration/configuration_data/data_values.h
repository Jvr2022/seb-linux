#pragma once

#include "../contracts/app_config.h"
#include "../contracts/session_configuration.h"
#include "../../settings/AppSettings.h"

#include <optional>

namespace seb::configuration::configuration_data {

class DataValues
{
public:
    contracts::AppConfig initializeAppConfig() const;
    contracts::SessionConfiguration initializeSessionConfiguration() const;
    seb::settings::AppSettings loadDefaultSettings() const;
    QString getAppDataFilePath() const;

private:
    mutable std::optional<contracts::AppConfig> appConfig_;
};

}  // namespace seb::configuration::configuration_data
