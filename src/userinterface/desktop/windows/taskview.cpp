#include "taskview.h"
#include "../controls/taskview/window_control.h"
#include <QVBoxLayout>
namespace seb::userinterface::desktop::windows { Taskview::Taskview(QWidget *parent) : BaseWindow(parent) { auto *l = new QVBoxLayout(this); l->addWidget(new controls::taskview::WindowControl(this)); } }
