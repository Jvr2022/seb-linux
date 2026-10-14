#pragma once

#include <QString>

namespace seb::settings::applications {

struct BlacklistApplication
{
    bool AutoTerminate = false;
    QString ExecutableName;
    int OperatingSystem = 0;
    QString OriginalName;
};

}  // namespace seb::settings::applications
