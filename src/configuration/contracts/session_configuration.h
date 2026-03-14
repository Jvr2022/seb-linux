#pragma once

#include "../../seb_settings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct SessionConfiguration
{
    AppConfig appConfig;
    QUuid clientAuthenticationToken;
    bool isBrowserResource = false;
    QUuid sessionId;
    seb::SebSettings settings;
};

}  // namespace seb::configuration::contracts
