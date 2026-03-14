#pragma once

#include "message.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::data {

struct PasswordReplyMessage : Message
{
    PasswordReplyMessage() = default;
    PasswordReplyMessage(QUuid id, bool ok, QString enteredPassword = {})
        : password(std::move(enteredPassword)), requestId(id), success(ok) {}

    QString password;
    QUuid requestId;
    bool success = false;
};

}  // namespace seb::communication::contracts::data
