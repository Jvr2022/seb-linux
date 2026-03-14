#pragma once

#include "../../seb_settings.h"

#include <QWidget>

namespace seb::shell::taskbar::platform {
class AudioController;
class BatteryController;
class KeyboardController;
class NetworkController;
}

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QScrollArea;
QT_END_NAMESPACE

class SebSession;

namespace seb::ui::taskbar {

class ApplicationControl;
class AudioControl;
class BatteryControl;
class ClockWidget;
class ExternalApplicationControl;
class KeyboardControl;
class NetworkControl;
class NotificationControl;
class RaiseHandControl;
class TaskbarButton;

class TaskbarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskbarWidget(SebSession &session, const seb::SebSettings &settings, QWidget *parent = nullptr);

signals:
    void quitRequested();

private:
    SebSession &session_;
    seb::shell::taskbar::platform::AudioController *audioController_ = nullptr;
    seb::shell::taskbar::platform::NetworkController *networkController_ = nullptr;
    seb::shell::taskbar::platform::BatteryController *batteryController_ = nullptr;
    seb::shell::taskbar::platform::KeyboardController *keyboardController_ = nullptr;
};

}  // namespace seb::ui::taskbar
