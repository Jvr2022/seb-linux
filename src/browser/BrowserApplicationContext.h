#pragma once

#include <QList>
#include <QString>

namespace seb::browser {

class BrowserWindow;

struct BrowserApplicationContext
{
    QList<BrowserWindow *> windows;
    QString startUrl;
};

}  // namespace seb::browser
