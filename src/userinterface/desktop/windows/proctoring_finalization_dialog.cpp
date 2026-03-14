#include "proctoring_finalization_dialog.h"
#include <QDialog>
namespace seb::userinterface::desktop::windows { ProctoringFinalizationDialog::ProctoringFinalizationDialog(QWidget *parent) : parent_(parent) {} int ProctoringFinalizationDialog::exec() { QDialog dialog(parent_); return dialog.exec(); } }
