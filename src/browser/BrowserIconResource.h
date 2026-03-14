#pragma once

#include <QString>

namespace seb::browser {

class BrowserIconResource
{
public:
    explicit BrowserIconResource(const QString &path = QStringLiteral(":/assets/icons/safe-exam-browser.png"))
        : path_(path)
    {
    }

    QString path() const { return path_; }

private:
    QString path_;
};

}  // namespace seb::browser
