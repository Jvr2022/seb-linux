#include "audio_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"
#include "../../../shell/taskbar/platform/audio_controller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

namespace {

QString iconForAudio(const seb::shell::taskbar::platform::AudioState &state)
{
    if (!state.available) return QStringLiteral(":/assets/taskbar/audio_no_device.svg");
    if (state.muted) return QStringLiteral(":/assets/taskbar/audio_muted.svg");
    if (state.volumePercent > 66) return QStringLiteral(":/assets/taskbar/audio_100.svg");
    if (state.volumePercent > 33) return QStringLiteral(":/assets/taskbar/audio_66.svg");
    return QStringLiteral(":/assets/taskbar/audio_33.svg");
}

}  // namespace

AudioControl::AudioControl(seb::shell::taskbar::platform::AudioController &controller, QWidget *parent)
    : QWidget(parent)
    , controller_(controller)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(40);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    auto *popupLayout = new QVBoxLayout(popup_);
    popupLayout->setContentsMargins(10, 10, 10, 10);

    deviceLabel_ = new QLabel(QStringLiteral("Audio Device"), popup_);
    deviceLabel_->setAlignment(Qt::AlignCenter);
    popupLayout->addWidget(deviceLabel_);

    auto *row = new QHBoxLayout();
    muteButton_ = new TaskbarButton(popup_);
    muteButton_->setFixedWidth(40);
    row->addWidget(muteButton_);

    slider_ = new QSlider(Qt::Horizontal, popup_);
    slider_->setRange(0, 100);
    slider_->setFixedWidth(250);
    row->addWidget(slider_);

    valueLabel_ = new QLabel(popup_);
    valueLabel_->setFixedWidth(40);
    row->addWidget(valueLabel_);
    popupLayout->addLayout(row);

    connect(button_, &QPushButton::clicked, this, [this] {
        button_->setHasPopupOpen(true);
        popup_->showAbove(button_);
    });
    connect(muteButton_, &QPushButton::clicked, this, [this] {
        controller_.setMuted(!controller_.state().muted);
    });
    connect(slider_, &QSlider::valueChanged, this, [this](int value) {
        controller_.setVolume(value);
    });
    connect(&controller_, &seb::shell::taskbar::platform::AudioController::stateChanged, this, &AudioControl::updateState);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    updateState();
}

void AudioControl::updateState()
{
    const auto &state = controller_.state();
    const QString tooltip = !state.available
        ? QStringLiteral("Audio device not found")
        : (state.muted
            ? QStringLiteral("Audio muted")
            : QStringLiteral("Audio volume: %1%").arg(state.volumePercent));

    button_->setEnabled(state.available);
    button_->setToolTip(tooltip);
    button_->setIconPath(iconForAudio(state));

    muteButton_->setEnabled(state.available);
    muteButton_->setToolTip(state.muted ? QStringLiteral("Unmute") : QStringLiteral("Mute"));
    muteButton_->setIconPath(iconForAudio(state));
    deviceLabel_->setText(state.available ? QStringLiteral("Default audio output") : QStringLiteral("Audio device not found"));

    slider_->blockSignals(true);
    slider_->setEnabled(state.available);
    slider_->setValue(qBound(0, state.volumePercent, 100));
    slider_->blockSignals(false);
    valueLabel_->setText(QStringLiteral("%1").arg(state.volumePercent));
}

}  // namespace seb::ui::taskbar
