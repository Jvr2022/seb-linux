#pragma once

#include <QWidget>

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class NotificationControl : public QWidget
{
    Q_OBJECT

public:
    NotificationControl(
        const QString &iconPath,
        const QString &toolTip,
        bool enabled,
        bool showDisabledTooltip = true,
        QWidget *parent = nullptr);

private:
    TaskbarButton *button_ = nullptr;
};

}  // namespace seb::ui::taskbar
