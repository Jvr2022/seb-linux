#include "log_view_model.h"

namespace seb::userinterface::desktop::viewmodels {

void LogViewModel::append(const QString &line)
{
    lines_.push_back(line);
}

QStringList LogViewModel::lines() const
{
    return lines_;
}

}  // namespace seb::userinterface::desktop::viewmodels
