#pragma once

#include "message.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::data {

struct ServerFailureActionRequestMessage : Message
{
    ServerFailureActionRequestMessage() = default;
    ServerFailureActionRequestMessage(QString requestedMessage, bool show, QUuid id)
        : message(std::move(requestedMessage)), showFallback(show), requestId(id) {}

    QString message;
    bool showFallback = false;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::data
