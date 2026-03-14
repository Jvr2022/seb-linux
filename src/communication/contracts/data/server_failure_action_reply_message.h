#pragma once

#include "message.h"

#include <QUuid>

namespace seb::communication::contracts::data {

struct ServerFailureActionReplyMessage : Message
{
    ServerFailureActionReplyMessage() = default;
    ServerFailureActionReplyMessage(bool shouldAbort, bool shouldFallback, bool shouldRetry, QUuid id)
        : abort(shouldAbort), fallback(shouldFallback), retry(shouldRetry), requestId(id) {}

    bool abort = false;
    bool fallback = false;
    bool retry = false;
    QUuid requestId;
};

}  // namespace seb::communication::contracts::data
