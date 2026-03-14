#pragma once

#include "Proxy/ProxyConfiguration.h"
#include "Proxy/ProxyPolicy.h"

#include <QList>
#include <QString>
#include <QStringList>

namespace seb::settings::browser {

struct ProxySettings
{
    bool AutoConfigure = false;
    bool AutoDetect = false;
    QString AutoConfigureUrl;
    QStringList BypassList;
    proxy::ProxyPolicy Policy = proxy::ProxyPolicy::System;
    QList<proxy::ProxyConfiguration> Proxies;
};

}  // namespace seb::settings::browser
