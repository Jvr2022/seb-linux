#include "server_failure_dialog.h"
#include <QDialog>
namespace seb::userinterface::desktop::windows { ServerFailureDialog::ServerFailureDialog(QWidget *parent) : parent_(parent) {} contracts::windows::data::ServerFailureDialogResult ServerFailureDialog::exec() { QDialog dialog(parent_); return dialog.exec() == QDialog::Accepted ? contracts::windows::data::ServerFailureDialogResult::Retry : contracts::windows::data::ServerFailureDialogResult::Quit; } }
