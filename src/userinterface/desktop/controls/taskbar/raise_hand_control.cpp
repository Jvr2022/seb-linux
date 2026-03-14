#include "raise_hand_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskbar { RaiseHandControl::RaiseHandControl(QWidget *parent) : TaskbarControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Raise Hand"), this)); } }
