#pragma once

#include <QString>

namespace seb::userinterface::contracts::windows::data {
struct ExamSelectionDialogResult { bool accepted = false; QString examId; };
}
