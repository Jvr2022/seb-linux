#include "message_box_factory.h"

#include <QMessageBox>

namespace seb::userinterface::desktop {

contracts::message_box::MessageBoxResult MessageBoxFactory::show(
    const QString &title,
    const QString &text,
    contracts::message_box::MessageBoxAction action,
    contracts::message_box::MessageBoxIcon icon)
{
    QMessageBox box;
    box.setWindowTitle(title);
    box.setText(text);
    box.setIcon(icon == contracts::message_box::MessageBoxIcon::Warning ? QMessageBox::Warning :
                icon == contracts::message_box::MessageBoxIcon::Error ? QMessageBox::Critical :
                icon == contracts::message_box::MessageBoxIcon::Question ? QMessageBox::Question :
                QMessageBox::Information);
    box.setStandardButtons(action == contracts::message_box::MessageBoxAction::YesNo ? QMessageBox::Yes | QMessageBox::No :
                           action == contracts::message_box::MessageBoxAction::OkCancel ? QMessageBox::Ok | QMessageBox::Cancel :
                           QMessageBox::Ok);
    const auto result = static_cast<QMessageBox::StandardButton>(box.exec());
    if (result == QMessageBox::Yes) return contracts::message_box::MessageBoxResult::Yes;
    if (result == QMessageBox::No) return contracts::message_box::MessageBoxResult::No;
    if (result == QMessageBox::Cancel) return contracts::message_box::MessageBoxResult::Cancel;
    return contracts::message_box::MessageBoxResult::Ok;
}

}  // namespace seb::userinterface::desktop
