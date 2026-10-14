#pragma once

#include "../../settings/AppSettings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct ServiceConfiguration
{
    AppConfig appConfig;
    QUuid sessionId;
    seb::settings::AppSettings settings;
    QString userName;
    QString userSid;
};

}  // namespace seb::configuration::contracts
