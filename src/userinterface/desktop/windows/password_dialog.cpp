#include "password_dialog.h"

#include <QDialogButtonBox>
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>

namespace seb::userinterface::desktop::windows {

PasswordDialog::PasswordDialog(QWidget *parent) : parent_(parent) {}

contracts::windows::data::PasswordDialogResult PasswordDialog::exec()
{
    QDialog dialog(parent_);
    dialog.setWindowTitle(QStringLiteral("Password"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    contracts::windows::data::PasswordDialogResult result;
    result.accepted = (dialog.exec() == QDialog::Accepted);
    result.password = passwordEdit->text();
    return result;
}

}  // namespace seb::userinterface::desktop::windows
