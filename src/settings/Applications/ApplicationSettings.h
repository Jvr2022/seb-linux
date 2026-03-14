#pragma once

#include "BlacklistApplication.h"
#include "WhitelistApplication.h"

#include <QList>

namespace seb::settings::applications {

struct ApplicationSettings
{
    QList<BlacklistApplication> Blacklist;
    QList<WhitelistApplication> Whitelist;
};

}  // namespace seb::settings::applications
