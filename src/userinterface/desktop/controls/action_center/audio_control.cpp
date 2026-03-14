#include "audio_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::action_center { AudioControl::AudioControl(QWidget *parent) : ActionCenterControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Audio"), this)); } }
