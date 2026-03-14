#pragma once

#include "message.h"

#include <QUuid>

namespace seb::communication::contracts::data {

struct SessionStopMessage : Message
{
    explicit SessionStopMessage(QUuid id = {})
        : sessionId(id) {}

    QUuid sessionId;
};

}  // namespace seb::communication::contracts::data
