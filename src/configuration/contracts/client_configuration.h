#pragma once

#include "../../seb_settings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct ClientConfiguration
{
    AppConfig appConfig;
    QUuid sessionId;
    seb::SebSettings settings;
};

}  // namespace seb::configuration::contracts
