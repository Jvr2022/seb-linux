#pragma once

#include <QString>

namespace seb::browser {

struct BrowserWindowContext
{
    int id = 0;
    bool isMainWindow = false;
    QString title;
    QString url;
};

}  // namespace seb::browser
