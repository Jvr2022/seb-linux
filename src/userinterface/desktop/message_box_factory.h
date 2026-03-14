#pragma once

#include "../contracts/message_box/i_message_box.h"

namespace seb::userinterface::desktop {

class MessageBoxFactory : public contracts::message_box::IMessageBox
{
public:
    contracts::message_box::MessageBoxResult show(
        const QString &title,
        const QString &text,
        contracts::message_box::MessageBoxAction action = contracts::message_box::MessageBoxAction::Ok,
        contracts::message_box::MessageBoxIcon icon = contracts::message_box::MessageBoxIcon::Information) override;
};

}  // namespace seb::userinterface::desktop
