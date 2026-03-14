#include "battery_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"
#include "../../../shell/taskbar/platform/battery_controller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

BatteryControl::BatteryControl(seb::shell::taskbar::platform::BatteryController &controller, QWidget *parent)
    : QWidget(parent)
    , controller_(controller)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    auto *popupLayout = new QVBoxLayout(popup_);
    popupLayout->setContentsMargins(20, 10, 20, 20);
    popupLayout->setSpacing(10);

    auto *topRow = new QHBoxLayout();
    topRow->addStretch(1);
    auto *closeButton = new QPushButton(QStringLiteral("X"), popup_);
    closeButton->setFixedWidth(20);
    closeButton->setFlat(true);
    closeButton->setCursor(Qt::PointingHandCursor);
    topRow->addWidget(closeButton);
    popupLayout->addLayout(topRow);

    auto *textLabel = new QLabel(popup_);
    textLabel->setWordWrap(true);
    popupLayout->addWidget(textLabel);
    connect(closeButton, &QPushButton::clicked, popup_, &QWidget::hide);

    connect(button_, &QPushButton::clicked, this, [this] {
        button_->setHasPopupOpen(true);
        popup_->showAbove(button_);
    });
    connect(&controller_, &seb::shell::taskbar::platform::BatteryController::stateChanged, this, &BatteryControl::updateState);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    updateState();
}

void BatteryControl::updateState()
{
    const auto &state = controller_.state();
    QString tooltip = QStringLiteral("Battery unavailable");
    if (state.available) {
        tooltip = state.charging
            ? QStringLiteral("Charging: %1%").arg(state.percentage)
            : QStringLiteral("Battery remaining: %1%").arg(state.percentage);
        if (!state.timeRemaining.isEmpty()) {
            tooltip += QStringLiteral(" (%1)").arg(state.timeRemaining);
        }
    }

    button_->setEnabled(state.available);
    button_->setToolTip(tooltip);
    button_->setIconPath(QStringLiteral(":/assets/taskbar/battery.svg"));
    if (auto *label = popup_->findChild<QLabel *>()) {
        label->setText(tooltip);
    }
}

}  // namespace seb::ui::taskbar
