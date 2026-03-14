#include "taskbar_widget.h"

#include "common/taskbar_button.h"
#include "common/taskbar_style.h"
#include "controls/application_control.h"
#include "controls/audio_control.h"
#include "controls/battery_control.h"
#include "controls/clock_widget.h"
#include "controls/external_application_control.h"
#include "controls/keyboard_control.h"
#include "controls/network_control.h"
#include "controls/notification_control.h"
#include "controls/raise_hand_control.h"

#include "../../shell/taskbar/platform/audio_controller.h"
#include "../../shell/taskbar/platform/battery_controller.h"
#include "../../shell/taskbar/platform/keyboard_controller.h"
#include "../../shell/taskbar/platform/network_controller.h"
#include "../../applications/external_application.h"
#include "../../shell/taskbar/taskbar_service.h"
#include "../../seb_session.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QScrollArea>

namespace seb::ui::taskbar {

TaskbarWidget::TaskbarWidget(SebSession &session, const seb::SebSettings &settings, QWidget *parent)
    : QWidget(parent)
    , session_(session)
{
    setFixedHeight(kTaskbarHeight);
    setAutoFillBackground(true);
    setStyleSheet(QStringLiteral("background:%1; color:black;").arg(backgroundColor().name()));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFixedHeight(kTaskbarHeight);
    scrollArea->setStyleSheet(
        "QScrollArea { background: #f0f0f0; border: 0; }"
        "QScrollBar:horizontal { background: #d8d8d8; height: 8px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: #b0b0b0; min-width: 20px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }");
    layout->addWidget(scrollArea, 1);

    auto *applicationContainer = new QWidget(scrollArea);
    auto *applicationLayout = new QHBoxLayout(applicationContainer);
    applicationLayout->setContentsMargins(0, 0, 0, 0);
    applicationLayout->setSpacing(0);
    applicationLayout->addWidget(new ApplicationControl(session_, applicationContainer));
    for (auto *application : session_.externalApplications()) {
        if (application->showInShell()) {
            applicationLayout->addWidget(new ExternalApplicationControl(*application, applicationContainer));
        }
    }
    applicationLayout->addStretch(1);
    scrollArea->setWidget(applicationContainer);

    audioController_ = new seb::shell::taskbar::platform::AudioController(this);
    networkController_ = new seb::shell::taskbar::platform::NetworkController(this);
    batteryController_ = new seb::shell::taskbar::platform::BatteryController(this);
    keyboardController_ = new seb::shell::taskbar::platform::KeyboardController(this);

    const auto model = seb::shell::taskbar::TaskbarService::buildModel(settings, batteryController_->state().available);

    auto *notificationContainer = new QWidget(this);
    auto *notificationLayout = new QHBoxLayout(notificationContainer);
    notificationLayout->setContentsMargins(0, 0, 0, 0);
    notificationLayout->setSpacing(0);
    if (model.showApplicationInfo) {
        notificationLayout->addWidget(new NotificationControl(
            QStringLiteral(":/assets/taskbar/about.svg"),
            QStringLiteral("Application information"),
            false,
            notificationContainer));
    }
    if (model.showApplicationLog) {
        notificationLayout->addWidget(new NotificationControl(
            QStringLiteral(":/assets/taskbar/log.svg"),
            QStringLiteral("Application log"),
            false,
            notificationContainer));
    }
    if (model.showVerificator) {
        notificationLayout->addWidget(new NotificationControl(
            QStringLiteral(":/assets/taskbar/verificator.svg"),
            QStringLiteral("SEB Verificator"),
            false,
            notificationContainer));
    }
    if (model.showProctoring) {
        notificationLayout->addWidget(new RaiseHandControl(notificationContainer));
    }
    layout->addWidget(notificationContainer);

    auto *systemContainer = new QWidget(this);
    auto *systemLayout = new QHBoxLayout(systemContainer);
    systemLayout->setContentsMargins(0, 0, 0, 0);
    systemLayout->setSpacing(0);
    if (model.showPower) {
        systemLayout->addWidget(new BatteryControl(*batteryController_, systemContainer));
    }
    if (model.showNetwork) {
        systemLayout->addWidget(new NetworkControl(*networkController_, systemContainer));
    }
    if (model.showAudio) {
        systemLayout->addWidget(new AudioControl(*audioController_, systemContainer));
    }
    if (model.showKeyboardLayout) {
        systemLayout->addWidget(new KeyboardControl(*keyboardController_, systemContainer));
    }
    layout->addWidget(systemContainer);

    if (model.showClock) {
        layout->addWidget(new ClockWidget(this));
    }

    if (model.showQuit) {
        auto *quitButton = new TaskbarButton(this);
        quitButton->setFixedWidth(40);
        quitButton->setToolTip(QStringLiteral("Quit Safe Exam Browser"));
        quitButton->setIconPath(QStringLiteral(":/assets/taskbar/shutdown.svg"));
        connect(quitButton, &QPushButton::clicked, this, &TaskbarWidget::quitRequested);
        layout->addWidget(quitButton);
    }
}

}  // namespace seb::ui::taskbar
