#pragma once

#include <QString>

namespace seb::settings::applications {

struct BlacklistApplication
{
    bool AutoTerminate = false;
    QString ExecutableName;
    QString OriginalName;
};

}  // namespace seb::settings::applications
