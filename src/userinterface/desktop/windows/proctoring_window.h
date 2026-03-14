#pragma once
#include "../../contracts/proctoring/i_proctoring_window.h"
#include "base_window.h"
namespace seb::userinterface::desktop::windows { class ProctoringWindow : public BaseWindow, public contracts::proctoring::IProctoringWindow { Q_OBJECT public: explicit ProctoringWindow(QWidget *parent = nullptr); void hide() override; void show() override; }; }
