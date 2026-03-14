#pragma once
#include "../../contracts/proctoring/i_proctoring_finalization_dialog.h"
class QWidget;
namespace seb::userinterface::desktop::windows { class ProctoringFinalizationDialog : public contracts::proctoring::IProctoringFinalizationDialog { public: explicit ProctoringFinalizationDialog(QWidget *parent = nullptr); int exec() override; private: QWidget *parent_ = nullptr; }; }
