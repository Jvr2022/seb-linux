#pragma once

#include <QWidget>

namespace seb::shell::taskbar::platform {
class NetworkController;
}

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class NetworkControl : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkControl(seb::shell::taskbar::platform::NetworkController &controller, QWidget *parent = nullptr);

private:
    void rebuildPopup();
    void updateState();

    seb::shell::taskbar::platform::NetworkController &controller_;
    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QVBoxLayout *popupLayout_ = nullptr;
};

}  // namespace seb::ui::taskbar
