#include "taskbar_window.h"
#include "../controls/taskbar/application_control.h"
#include "../controls/taskbar/audio_control.h"
#include "../controls/taskbar/network_control.h"
#include "../controls/taskbar/power_supply_control.h"
#include "../controls/taskbar/clock.h"
#include "../controls/taskbar/quit_button.h"
#include <QHBoxLayout>
namespace seb::userinterface::desktop::windows { TaskbarWindow::TaskbarWindow(QWidget *parent) : BaseWindow(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new controls::taskbar::ApplicationControl(this)); l->addWidget(new controls::taskbar::PowerSupplyControl(this)); l->addWidget(new controls::taskbar::NetworkControl(this)); l->addWidget(new controls::taskbar::AudioControl(this)); l->addWidget(new controls::taskbar::Clock(this)); l->addWidget(new controls::taskbar::QuitButton(this)); } }
