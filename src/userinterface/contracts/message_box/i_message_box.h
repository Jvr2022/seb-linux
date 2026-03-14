#pragma once

#include "message_box_action.h"
#include "message_box_icon.h"
#include "message_box_result.h"

#include <QString>

namespace seb::userinterface::contracts::message_box {

class IMessageBox
{
public:
    virtual ~IMessageBox() = default;
    virtual MessageBoxResult show(
        const QString &title,
        const QString &text,
        MessageBoxAction action = MessageBoxAction::Ok,
        MessageBoxIcon icon = MessageBoxIcon::Information) = 0;
};

}  // namespace seb::userinterface::contracts::message_box
