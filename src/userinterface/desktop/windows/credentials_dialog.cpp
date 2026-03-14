#include "credentials_dialog.h"

#include <QDialogButtonBox>
#include <QDialog>
#include <QLineEdit>
#include <QVBoxLayout>

namespace seb::userinterface::desktop::windows {

CredentialsDialog::CredentialsDialog(QWidget *parent) : parent_(parent) {}

contracts::windows::data::CredentialsDialogResult CredentialsDialog::exec()
{
    QDialog dialog(parent_);
    dialog.setWindowTitle(QStringLiteral("Credentials"));
    auto *layout = new QVBoxLayout(&dialog);
    auto *usernameEdit = new QLineEdit(&dialog);
    auto *passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(usernameEdit);
    layout->addWidget(passwordEdit);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    contracts::windows::data::CredentialsDialogResult result;
    result.accepted = (dialog.exec() == QDialog::Accepted);
    result.username = usernameEdit->text();
    result.password = passwordEdit->text();
    return result;
}

}  // namespace seb::userinterface::desktop::windows
