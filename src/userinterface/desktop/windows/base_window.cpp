#include "base_window.h"

namespace seb::userinterface::desktop::windows {

BaseWindow::BaseWindow(QWidget *parent) : QDialog(parent) {}
void BaseWindow::show() { QDialog::show(); }
void BaseWindow::hide() { QDialog::hide(); }
void BaseWindow::close() { QDialog::close(); }

}  // namespace seb::userinterface::desktop::windows
