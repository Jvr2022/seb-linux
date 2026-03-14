#include "keyboard_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"
#include "../../../shell/taskbar/platform/keyboard_controller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

KeyboardControl::KeyboardControl(seb::shell::taskbar::platform::KeyboardController &controller, QWidget *parent)
    : QWidget(parent)
    , controller_(controller)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    button_->setTextMode(true);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    popupLayout_ = new QVBoxLayout(popup_);
    popupLayout_->setContentsMargins(0, 0, 0, 0);
    popupLayout_->setSpacing(0);

    connect(button_, &QPushButton::clicked, this, [this] {
        button_->setHasPopupOpen(true);
        rebuildPopup();
        popup_->showAbove(button_);
    });
    connect(&controller_, &seb::shell::taskbar::platform::KeyboardController::stateChanged, this, &KeyboardControl::updateState);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    updateState();
}

void KeyboardControl::rebuildPopup()
{
    QLayoutItem *item = nullptr;
    while ((item = popupLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    const auto &state = controller_.state();
    if (state.availableLayouts.isEmpty()) {
        auto *label = new QLabel(QStringLiteral("No layout information available."), popup_);
        label->setContentsMargins(10, 10, 10, 10);
        popupLayout_->addWidget(label);
        return;
    }

    for (const QString &layout : state.availableLayouts) {
        auto *button = new QPushButton(popup_);
        button->setFlat(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setMinimumHeight(40);
        button->setStyleSheet(QStringLiteral("text-align:left; padding:10px 10px; border:none;"));
        const QString label = QStringLiteral("%1  %2")
                                  .arg(layout.compare(state.currentLongName, Qt::CaseInsensitive) == 0 ||
                                           layout.compare(state.currentShortName, Qt::CaseInsensitive) == 0
                                           ? QStringLiteral("•")
                                           : QStringLiteral(" "))
                                  .arg(layout.toUpper());
        button->setText(label);
        connect(button, &QPushButton::clicked, popup_, &QWidget::hide);
        popupLayout_->addWidget(button);
    }
}

void KeyboardControl::updateState()
{
    const auto &state = controller_.state();
    button_->setText(state.currentShortName.isEmpty() ? QStringLiteral("--") : state.currentShortName.left(3));
    button_->setToolTip(state.currentLongName.isEmpty() ? QStringLiteral("Keyboard layout unavailable") : state.currentLongName);
    button_->setEnabled(!state.currentShortName.isEmpty());
}

}  // namespace seb::ui::taskbar
