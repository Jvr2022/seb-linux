#pragma once

#include "../contracts/file_system_dialog/i_file_system_dialog.h"

#include <memory>

namespace seb::userinterface::desktop {

class FileSystemDialogFactory : public contracts::file_system_dialog::IFileSystemDialog
{
public:
    FileSystemDialogFactory() = default;

    contracts::file_system_dialog::FileSystemDialogResult exec() override;
    QString selectedPath() const override;
    void setOperation(contracts::file_system_dialog::FileSystemOperation operation) override;

private:
    QString selectedPath_;
    contracts::file_system_dialog::FileSystemOperation operation_ =
        contracts::file_system_dialog::FileSystemOperation::Open;
};

}  // namespace seb::userinterface::desktop
