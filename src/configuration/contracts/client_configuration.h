#pragma once

#include "../../settings/AppSettings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct ClientConfiguration
{
    AppConfig appConfig;
    QUuid sessionId;
    seb::settings::AppSettings settings;
};

}  // namespace seb::configuration::contracts
