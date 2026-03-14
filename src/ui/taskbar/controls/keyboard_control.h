#pragma once

#include <QWidget>

namespace seb::shell::taskbar::platform {
class KeyboardController;
}

QT_BEGIN_NAMESPACE
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class KeyboardControl : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardControl(seb::shell::taskbar::platform::KeyboardController &controller, QWidget *parent = nullptr);

private:
    void rebuildPopup();
    void updateState();

    seb::shell::taskbar::platform::KeyboardController &controller_;
    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QVBoxLayout *popupLayout_ = nullptr;
};

}  // namespace seb::ui::taskbar
