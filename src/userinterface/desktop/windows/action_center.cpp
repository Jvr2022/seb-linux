#include "action_center.h"
#include "../controls/action_center/application_control.h"
#include "../controls/action_center/audio_control.h"
#include "../controls/action_center/network_control.h"
#include "../controls/action_center/power_supply_control.h"
#include "../controls/action_center/clock.h"
#include "../controls/action_center/quit_button.h"
#include <QVBoxLayout>
namespace seb::userinterface::desktop::windows { ActionCenter::ActionCenter(QWidget *parent) : BaseWindow(parent) { auto *l = new QVBoxLayout(this); l->addWidget(new controls::action_center::ApplicationControl(this)); l->addWidget(new controls::action_center::AudioControl(this)); l->addWidget(new controls::action_center::NetworkControl(this)); l->addWidget(new controls::action_center::PowerSupplyControl(this)); l->addWidget(new controls::action_center::Clock(this)); l->addWidget(new controls::action_center::QuitButton(this)); } }
