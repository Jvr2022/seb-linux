#include "notification_control.h"

#include "../common/taskbar_button.h"

#include <QHBoxLayout>

namespace seb::ui::taskbar {

NotificationControl::NotificationControl(
    const QString &iconPath,
    const QString &toolTip,
    bool enabled,
    bool showDisabledTooltip,
    QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    button_->setIconPath(iconPath);
    button_->setToolTip(toolTip);
    button_->setEnabled(enabled);
    button_->setAttribute(Qt::WA_AlwaysShowToolTips, showDisabledTooltip);
    layout->addWidget(button_);
}

}  // namespace seb::ui::taskbar
