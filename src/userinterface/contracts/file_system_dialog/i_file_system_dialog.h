#pragma once

#include "file_system_dialog_result.h"
#include "file_system_operation.h"

#include <QString>

namespace seb::userinterface::contracts::file_system_dialog {

class IFileSystemDialog
{
public:
    virtual ~IFileSystemDialog() = default;
    virtual FileSystemDialogResult exec() = 0;
    virtual QString selectedPath() const = 0;
    virtual void setOperation(FileSystemOperation operation) = 0;
};

}  // namespace seb::userinterface::contracts::file_system_dialog
