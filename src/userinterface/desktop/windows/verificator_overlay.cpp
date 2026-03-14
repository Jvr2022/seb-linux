#include "verificator_overlay.h"

namespace seb::userinterface::desktop::windows {

VerificatorOverlay::VerificatorOverlay(QWidget *parent) : BaseWindow(parent) {}

void VerificatorOverlay::show()
{
    BaseWindow::show();
}

void VerificatorOverlay::hide()
{
    BaseWindow::hide();
}

void VerificatorOverlay::close()
{
    BaseWindow::close();
}

}  // namespace seb::userinterface::desktop::windows
