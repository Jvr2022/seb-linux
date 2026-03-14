#pragma once
#include "../../contracts/file_system_dialog/i_file_system_dialog.h"
class QWidget;
namespace seb::userinterface::desktop::windows { class FileSystemDialog : public contracts::file_system_dialog::IFileSystemDialog { public: explicit FileSystemDialog(QWidget *parent = nullptr); contracts::file_system_dialog::FileSystemDialogResult exec() override; QString selectedPath() const override; void setOperation(contracts::file_system_dialog::FileSystemOperation operation) override; private: QWidget *parent_ = nullptr; QString selectedPath_; contracts::file_system_dialog::FileSystemOperation operation_ = contracts::file_system_dialog::FileSystemOperation::Open; }; }
