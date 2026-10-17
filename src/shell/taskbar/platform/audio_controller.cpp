#include "audio_controller.h"

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
    if (getVolumeProcess_ && getVolumeProcess_->state() != QProcess::NotRunning) {
        return;
    }

    if (!getVolumeProcess_) {
        getVolumeProcess_ = new QProcess(this);
        connect(getVolumeProcess_, &QProcess::finished, this, &AudioController::onGetVolumeFinished);
    }

    getVolumeProcess_->start(QStringLiteral("wpctl"), {QStringLiteral("get-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@")});
}

void AudioController::onGetVolumeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    AudioState next;
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        next.available = true;
        const QString output = QString::fromUtf8(getVolumeProcess_->readAllStandardOutput()).trimmed();
        next.muted = output.contains(QStringLiteral("[MUTED]"), Qt::CaseInsensitive);
        const QRegularExpression regex(QStringLiteral("([0-9]+(?:\\.[0-9]+)?)"));
        const auto match = regex.match(output);
        if (match.hasMatch()) {
            next.volumePercent = qBound(0, qRound(match.captured(1).toDouble() * 100.0), 100);
        }
    } else {
        next.available = false;
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
    
    QProcess::startDetached(QStringLiteral("wpctl"), {QStringLiteral("set-mute"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), muted ? QStringLiteral("1") : QStringLiteral("0")});
    
    if (state_.muted != muted) {
        state_.muted = muted;
        emit stateChanged();
    }
}

void AudioController::setVolume(int volumePercent)
{
    if (!state_.available) {
        return;
    }
    const int bounded = qBound(0, volumePercent, 100);
    
    QProcess::startDetached(QStringLiteral("wpctl"), {QStringLiteral("set-volume"), QStringLiteral("@DEFAULT_AUDIO_SINK@"), QStringLiteral("%1%").arg(bounded)});
    
    if (state_.volumePercent != bounded) {
        state_.volumePercent = bounded;
        emit stateChanged();
    }
}

}  // namespace seb::shell::taskbar::platform
