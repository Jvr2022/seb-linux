#pragma once

#include <QObject>
#include <QProcess>

QT_BEGIN_NAMESPACE
class QTimer;
QT_END_NAMESPACE

namespace seb::shell::taskbar::platform {

struct AudioState
{
    bool available = false;
    bool muted = false;
    int volumePercent = 0;
};

class AudioController : public QObject
{
    Q_OBJECT

public:
    explicit AudioController(QObject *parent = nullptr);

    const AudioState &state() const;
    void refresh();
    void setMuted(bool muted);
    void setVolume(int volumePercent);

signals:
    void stateChanged();

private slots:
    void onGetVolumeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    AudioState state_;
    QTimer *timer_ = nullptr;
    QProcess *getVolumeProcess_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
