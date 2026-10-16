#pragma once

#include <QString>

namespace seb::server::data {

struct ConnectionInfo
{
    QString api;
    QString connectionToken;
    QString oauth2Token;
};

}  // namespace seb::server::data
