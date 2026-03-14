#include "exam_selection_dialog.h"
#include <QDialogButtonBox>
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
namespace seb::userinterface::desktop::windows { ExamSelectionDialog::ExamSelectionDialog(QWidget *parent) : parent_(parent) {} contracts::windows::data::ExamSelectionDialogResult ExamSelectionDialog::exec() { QDialog dialog(parent_); auto *l = new QVBoxLayout(&dialog); auto *list = new QListWidget(&dialog); l->addWidget(list); auto *b = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog); QObject::connect(b, &QDialogButtonBox::accepted, &dialog, &QDialog::accept); QObject::connect(b, &QDialogButtonBox::rejected, &dialog, &QDialog::reject); l->addWidget(b); contracts::windows::data::ExamSelectionDialogResult r; r.accepted = (dialog.exec() == QDialog::Accepted); if (auto *item = list->currentItem()) r.examId = item->text(); return r; } }
