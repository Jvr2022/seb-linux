#pragma once

#include <QDateTime>

namespace seb::userinterface::desktop::viewmodels {

class DateTimeViewModel
{
public:
    QDateTime current() const;
};

}  // namespace seb::userinterface::desktop::viewmodels
