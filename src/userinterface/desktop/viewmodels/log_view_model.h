#pragma once

#include <QStringList>

namespace seb::userinterface::desktop::viewmodels {

class LogViewModel
{
public:
    void append(const QString &line);
    QStringList lines() const;

private:
    QStringList lines_;
};

}  // namespace seb::userinterface::desktop::viewmodels
