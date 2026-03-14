#include "window_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskview { WindowControl::WindowControl(QWidget *parent) : QWidget(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Window"), this)); } }
