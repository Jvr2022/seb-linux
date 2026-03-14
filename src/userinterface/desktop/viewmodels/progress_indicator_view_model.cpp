#include "progress_indicator_view_model.h"

namespace seb::userinterface::desktop::viewmodels {

void ProgressIndicatorViewModel::setBusy(bool busy) { busy_ = busy; }
void ProgressIndicatorViewModel::setProgress(int current, int maximum) { current_ = current; maximum_ = maximum; }
bool ProgressIndicatorViewModel::busy() const { return busy_; }
int ProgressIndicatorViewModel::current() const { return current_; }
int ProgressIndicatorViewModel::maximum() const { return maximum_; }

}  // namespace seb::userinterface::desktop::viewmodels
