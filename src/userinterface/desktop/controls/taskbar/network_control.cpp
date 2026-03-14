#include "network_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskbar { NetworkControl::NetworkControl(QWidget *parent) : TaskbarControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Network"), this)); } }
