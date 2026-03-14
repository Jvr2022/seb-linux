#include "clock.h"
#include <QVBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { Clock::Clock(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QVBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("00:00"), this)); l->addWidget(new QLabel(QStringLiteral("1970-01-01"), this)); } }
