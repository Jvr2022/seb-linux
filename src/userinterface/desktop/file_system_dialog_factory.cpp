#include "file_system_dialog_factory.h"

#include <QFileDialog>

namespace seb::userinterface::desktop {

using contracts::file_system_dialog::FileSystemDialogResult;
using contracts::file_system_dialog::FileSystemOperation;

FileSystemDialogResult FileSystemDialogFactory::exec()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::AnyFile);

    if (operation_ == FileSystemOperation::Save) {
        dialog.setAcceptMode(QFileDialog::AcceptSave);
    } else {
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
    }

    if (dialog.exec() == QDialog::Accepted) {
        const QStringList files = dialog.selectedFiles();
        if (!files.isEmpty()) {
            selectedPath_ = files.first();
        }
        return FileSystemDialogResult::Accept;
    }

    selectedPath_.clear();
    return FileSystemDialogResult::Cancel;
}

QString FileSystemDialogFactory::selectedPath() const
{
    return selectedPath_;
}

void FileSystemDialogFactory::setOperation(FileSystemOperation operation)
{
    operation_ = operation;
}

}  // namespace seb::userinterface::desktop
