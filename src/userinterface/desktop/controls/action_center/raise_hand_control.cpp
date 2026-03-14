#include "raise_hand_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { RaiseHandControl::RaiseHandControl(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Raise Hand"), this)); } }
