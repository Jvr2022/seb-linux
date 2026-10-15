#pragma once

#include "../../settings/AppSettings.h"
#include "app_config.h"

#include <QUuid>

namespace seb::configuration::contracts {

struct SessionConfiguration
{
    AppConfig appConfig;
    QString browserProfilePath;
    QUuid clientAuthenticationToken;
    bool isBrowserResource = false;
    bool reuseBrowserProfile = false;
    QUuid sessionId;
    seb::settings::AppSettings settings;
};

}  // namespace seb::configuration::contracts
