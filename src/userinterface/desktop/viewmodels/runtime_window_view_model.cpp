#include "runtime_window_view_model.h"

namespace seb::userinterface::desktop::viewmodels {

void RuntimeWindowViewModel::setStatus(const QString &status) { status_ = status; }
QString RuntimeWindowViewModel::status() const { return status_; }

}  // namespace seb::userinterface::desktop::viewmodels
