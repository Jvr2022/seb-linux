#pragma once

#include "../../contracts/windows/i_password_dialog.h"

class QWidget;

namespace seb::userinterface::desktop::windows {

class PasswordDialog : public contracts::windows::IPasswordDialog
{
public:
    explicit PasswordDialog(QWidget *parent = nullptr);
    contracts::windows::data::PasswordDialogResult exec() override;

private:
    QWidget *parent_ = nullptr;
};

}  // namespace seb::userinterface::desktop::windows
