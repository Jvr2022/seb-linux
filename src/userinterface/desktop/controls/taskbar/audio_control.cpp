#include "audio_control.h"
#include <QHBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::controls::taskbar { AudioControl::AudioControl(QWidget *parent) : TaskbarControlBase(parent) { auto *l = new QHBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Audio"), this)); } }
