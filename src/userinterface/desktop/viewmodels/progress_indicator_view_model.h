#pragma once

namespace seb::userinterface::desktop::viewmodels {

class ProgressIndicatorViewModel
{
public:
    void setBusy(bool busy);
    void setProgress(int current, int maximum);
    bool busy() const;
    int current() const;
    int maximum() const;

private:
    bool busy_ = false;
    int current_ = 0;
    int maximum_ = 0;
};

}  // namespace seb::userinterface::desktop::viewmodels
