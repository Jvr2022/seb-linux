#include "keyboard_layout_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { KeyboardLayoutControl::KeyboardLayoutControl(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Keyboard"), this)); } }
