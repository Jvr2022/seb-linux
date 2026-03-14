#pragma once

#include "data/exam_selection_dialog_result.h"

namespace seb::userinterface::contracts::windows { class IExamSelectionDialog { public: virtual ~IExamSelectionDialog() = default; virtual data::ExamSelectionDialogResult exec() = 0; }; }
