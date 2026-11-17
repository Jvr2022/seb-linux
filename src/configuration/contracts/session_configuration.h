#pragma once

#include "../../settings/AppSettings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct SessionConfiguration
{
    AppConfig appConfig;
    QUuid clientAuthenticationToken;
    bool isBrowserResource = false;
    QUuid sessionId;
    seb::settings::AppSettings settings;
};

}  // namespace seb::configuration::contracts
