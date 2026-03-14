#pragma once

#include "ProxyProtocol.h"

#include <QString>

namespace seb::settings::browser::proxy {

struct ProxyConfiguration
{
    QString Host;
    QString Password;
    int Port = 0;
    ProxyProtocol Protocol = ProxyProtocol::Http;
    bool RequiresAuthentication = false;
    QString Username;
};

}  // namespace seb::settings::browser::proxy
