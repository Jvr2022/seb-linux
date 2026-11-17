#include "raise_hand_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"

#include <QGridLayout>
#include <QPointer>
#include <QLineEdit>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

RaiseHandControl::RaiseHandControl(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    auto *popupLayout = new QVBoxLayout(popup_);
    popupLayout->setContentsMargins(0, 0, 0, 0);
    popupLayout->setSpacing(0);

    auto *container = new QWidget(popup_);
    auto *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(5, 5, 5, 5);
    containerLayout->setSpacing(5);

    messageEdit_ = new QLineEdit(container);
    messageEdit_->setMinimumWidth(350);
    messageEdit_->setPlaceholderText(QStringLiteral("Optional message"));
    containerLayout->addWidget(messageEdit_);

    auto *buttonRow = new QGridLayout();
    buttonRow->setContentsMargins(0, 0, 0, 0);
    actionButton_ = new TaskbarButton(container);
    actionButton_->setFixedSize(150, 30);
    actionButton_->setTextMode(true);
    buttonRow->addWidget(actionButton_, 0, 0, Qt::AlignHCenter);
    containerLayout->addLayout(buttonRow);

    popupLayout->addWidget(container);

    connect(button_, &QPushButton::clicked, this, [this] {
        if (popup_->isVisible()) {
            popup_->hide();
        } else {
            button_->setHasPopupOpen(true);
            popup_->showAbove(button_);
        }
    });
    connect(actionButton_, &QPushButton::clicked, this, &RaiseHandControl::toggleRaisedState);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    updateState();
}

void RaiseHandControl::updateState()
{
    button_->setIconPath(handRaised_
            ? QStringLiteral(":/assets/taskbar/hand_raised.svg")
            : QStringLiteral(":/assets/taskbar/hand_lowered.svg"));
    button_->setToolTip(handRaised_ ? QStringLiteral("Hand raised") : QStringLiteral("Raise hand"));
    actionButton_->setText(handRaised_ ? QStringLiteral("Lower Hand") : QStringLiteral("Raise Hand"));
    messageEdit_->setEnabled(!handRaised_);
    if (handRaised_) {
        popup_->hide();
    }
}

void RaiseHandControl::toggleRaisedState()
{
    if (pending_ || (!handRaised_ && messageRequired_ && messageEdit_->text().trimmed().isEmpty())) return;
    const bool raised = !handRaised_;
    const auto generation = ++generation_;
    pending_ = true;
    actionButton_->setEnabled(false);
    QPointer<RaiseHandControl> self(this);
    auto done = [self, raised, generation](bool success) {
        if (!self || generation != self->generation_) return;
        self->pending_ = false;
        self->actionButton_->setEnabled(true);
        if (success) {
            self->handRaised_ = raised;
            if (raised) self->messageEdit_->clear();
        }
        self->updateState();
    };
    if (raised && raiseCallback_) raiseCallback_(messageEdit_->text(), done);
    else if (!raised && lowerCallback_) lowerCallback_(done);
    else done(false);
}

void RaiseHandControl::setServerCallbacks(
    std::function<void(const QString &, std::function<void(bool)>)> raise,
    std::function<void(std::function<void(bool)>)> lower)
{
    raiseCallback_ = std::move(raise);
    lowerCallback_ = std::move(lower);
}

void RaiseHandControl::setMessageRequired(bool required)
{
    messageRequired_ = required;
    messageEdit_->setPlaceholderText(required
        ? QStringLiteral("Message required")
        : QStringLiteral("Optional message"));
}

void RaiseHandControl::resetRaisedState()
{
    ++generation_;
    pending_ = false;
    actionButton_->setEnabled(true);
    handRaised_ = false;
    updateState();
}

}  // namespace seb::ui::taskbar
