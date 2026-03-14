#pragma once

#include <QString>

namespace seb::userinterface::contracts::browser::data {

struct JavascriptResult
{
    bool success = false;
    QString value;
};

}  // namespace seb::userinterface::contracts::browser::data
