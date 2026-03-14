#pragma once

#include <QObject>

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

private:
    AudioState state_;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::shell::taskbar::platform
