#include "browser_window.h"
#include <QVBoxLayout>
#include <QLabel>
namespace seb::userinterface::desktop::windows { BrowserWindow::BrowserWindow(QWidget *parent) : BaseWindow(parent) { auto *l = new QVBoxLayout(this); l->addWidget(new QLabel(QStringLiteral("Browser Window"), this)); } }
