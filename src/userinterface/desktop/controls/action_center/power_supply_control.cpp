#include "power_supply_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { PowerSupplyControl::PowerSupplyControl(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Power"), this)); } }
