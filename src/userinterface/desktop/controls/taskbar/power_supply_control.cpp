#include "power_supply_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskbar { PowerSupplyControl::PowerSupplyControl(QWidget *parent) : TaskbarControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Power"), this)); } }
