#pragma once

#include "../windows/i_window.h"
#include "i_browser_control.h"

namespace seb::userinterface::contracts::browser {

class IBrowserWindow : public windows::IWindow
{
public:
    ~IBrowserWindow() override = default;
    virtual IBrowserControl *browserControl() = 0;
};

}  // namespace seb::userinterface::contracts::browser
