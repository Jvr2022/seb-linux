#pragma once

#include "message.h"

#include <QUuid>

namespace seb::communication::contracts::data {

struct MessageBoxReplyMessage : Message
{
    MessageBoxReplyMessage() = default;
    MessageBoxReplyMessage(QUuid id, int value)
        : requestId(id), result(value) {}

    QUuid requestId;
    int result = 0;
};

}  // namespace seb::communication::contracts::data
