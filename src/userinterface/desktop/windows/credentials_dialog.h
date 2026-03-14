#pragma once

#include "../../contracts/windows/i_credentials_dialog.h"

class QWidget;

namespace seb::userinterface::desktop::windows {

class CredentialsDialog : public contracts::windows::ICredentialsDialog
{
public:
    explicit CredentialsDialog(QWidget *parent = nullptr);
    contracts::windows::data::CredentialsDialogResult exec() override;

private:
    QWidget *parent_ = nullptr;
};

}  // namespace seb::userinterface::desktop::windows
