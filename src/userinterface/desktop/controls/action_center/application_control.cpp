#include "application_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { ApplicationControl::ApplicationControl(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Application"), this)); } }
