#include "date_time_view_model.h"

namespace seb::userinterface::desktop::viewmodels {

QDateTime DateTimeViewModel::current() const
{
    return QDateTime::currentDateTime();
}

}  // namespace seb::userinterface::desktop::viewmodels
