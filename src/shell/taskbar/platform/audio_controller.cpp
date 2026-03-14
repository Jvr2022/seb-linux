#include "audio_controller.h"

#include "command_helper.h"

#include <QRegularExpression>
#include <QTimer>

namespace seb::shell::taskbar::platform {

AudioController::AudioController(QObject *parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(3000);
    connect(timer_, &QTimer::timeout, this, &AudioController::refresh);
    timer_->start();
    refresh();
}

const AudioState &AudioController::state() const
{
    return state_;
}

void AudioController::refresh()
{
    AudioState next;
    const QString output = runCommand(QStringLiteral("wpctl"), {QStringLiteral("get-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@")});
    if (!output.isEmpty()) {
        next.available = true;
        next.muted = output.contains(QStringLiteral("[MUTED]"), Qt::CaseInsensitive);
        const QRegularExpression regex(QStringLiteral("([0-9]+(?:\\.[0-9]+)?)"));
        const auto match = regex.match(output);
        if (match.hasMatch()) {
            next.volumePercent = qBound(0, qRound(match.captured(1).toDouble() * 100.0), 100);
        }
    }

    if (next.available != state_.available || next.muted != state_.muted || next.volumePercent != state_.volumePercent) {
        state_ = next;
        emit stateChanged();
    }
}

void AudioController::setMuted(bool muted)
{
    if (!state_.available) {
        return;
    }
    runCommand(QStringLiteral("wpctl"), {QStringLiteral("set-mute"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), muted ? QStringLiteral("1") : QStringLiteral("0")});
    refresh();
}

void AudioController::setVolume(int volumePercent)
{
    if (!state_.available) {
        return;
    }
    const int bounded = qBound(0, volumePercent, 100);
    runCommand(QStringLiteral("wpctl"), {QStringLiteral("set-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), QStringLiteral("%1%").arg(bounded)});
    refresh();
}

}  // namespace seb::shell::taskbar::platform
