#include "application_window_button.h"
namespace seb::userinterface::desktop::controls::taskbar { ApplicationWindowButton::ApplicationWindowButton(QWidget *parent) : QPushButton(parent) { setText(QStringLiteral("Window")); } }
