#pragma once

#include "../../contracts/windows/i_splash_screen.h"

#include <QSplashScreen>

namespace seb::userinterface::desktop::windows {

class SplashScreen : public QSplashScreen, public contracts::windows::ISplashScreen
{
public:
    explicit SplashScreen(const QPixmap &pixmap = QPixmap());
    void close() override;
    void hide() override;
    void show() override;
};

}  // namespace seb::userinterface::desktop::windows
