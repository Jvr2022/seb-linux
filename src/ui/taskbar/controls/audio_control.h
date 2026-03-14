#pragma once

#include <QWidget>

namespace seb::shell::taskbar::platform {
class AudioController;
}

QT_BEGIN_NAMESPACE
class QLabel;
class QSlider;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class AudioControl : public QWidget
{
    Q_OBJECT

public:
    explicit AudioControl(seb::shell::taskbar::platform::AudioController &controller, QWidget *parent = nullptr);

private:
    void updateState();

    seb::shell::taskbar::platform::AudioController &controller_;
    TaskbarButton *button_ = nullptr;
    TaskbarButton *muteButton_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QLabel *deviceLabel_ = nullptr;
    QSlider *slider_ = nullptr;
    QLabel *valueLabel_ = nullptr;
};

}  // namespace seb::ui::taskbar
