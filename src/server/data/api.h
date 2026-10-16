#pragma once

#include <QString>

namespace seb::server::data {

struct Api
{
    QString accessTokenEndpoint;
    QString configurationEndpoint;
    QString handshakeEndpoint;
    QString logEndpoint;
    QString pingEndpoint;
};

}  // namespace seb::server::data
