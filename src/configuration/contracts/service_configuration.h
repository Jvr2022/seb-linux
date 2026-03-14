#pragma once

#include "../../seb_settings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct ServiceConfiguration
{
    AppConfig appConfig;
    QUuid sessionId;
    seb::SebSettings settings;
    QString userName;
    QString userSid;
};

}  // namespace seb::configuration::contracts
