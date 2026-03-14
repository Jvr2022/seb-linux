#pragma once
#include "../../contracts/windows/i_exam_selection_dialog.h"
class QWidget;
namespace seb::userinterface::desktop::windows { class ExamSelectionDialog : public contracts::windows::IExamSelectionDialog { public: explicit ExamSelectionDialog(QWidget *parent = nullptr); contracts::windows::data::ExamSelectionDialogResult exec() override; private: QWidget *parent_ = nullptr; }; }
