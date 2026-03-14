#pragma once

#include "base_window.h"

namespace seb::userinterface::desktop::windows {
class AboutWindow : public BaseWindow { Q_OBJECT public: explicit AboutWindow(QWidget *parent = nullptr); };
}
