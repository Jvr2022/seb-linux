#pragma once

#include "message.h"
#include "password_request_purpose.h"

#include <QUuid>

namespace seb::communication::contracts::data {

struct PasswordRequestMessage : Message
{
    PasswordRequestMessage() = default;
    PasswordRequestMessage(PasswordRequestPurpose requestedPurpose, QUuid id)
        : purpose(requestedPurpose), requestId(id) {}

    PasswordRequestPurpose purpose = PasswordRequestPurpose::Settings;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::data
