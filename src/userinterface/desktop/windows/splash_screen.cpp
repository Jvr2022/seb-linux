#include "splash_screen.h"

namespace seb::userinterface::desktop::windows {

SplashScreen::SplashScreen(const QPixmap &pixmap) : QSplashScreen(pixmap) {}
void SplashScreen::close() { QSplashScreen::close(); }
void SplashScreen::hide() { QSplashScreen::hide(); }
void SplashScreen::show() { QSplashScreen::show(); }

}  // namespace seb::userinterface::desktop::windows
