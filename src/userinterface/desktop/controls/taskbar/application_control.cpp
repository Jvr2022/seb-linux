#include "application_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskbar { ApplicationControl::ApplicationControl(QWidget *parent) : TaskbarControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("App"), this)); } }
