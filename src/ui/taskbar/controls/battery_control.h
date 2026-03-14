#pragma once

#include <QWidget>

namespace seb::shell::taskbar::platform {
class BatteryController;
}

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class BatteryControl : public QWidget
{
    Q_OBJECT

public:
    explicit BatteryControl(seb::shell::taskbar::platform::BatteryController &controller, QWidget *parent = nullptr);

private:
    void updateState();

    seb::shell::taskbar::platform::BatteryController &controller_;
    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
};

}  // namespace seb::ui::taskbar
