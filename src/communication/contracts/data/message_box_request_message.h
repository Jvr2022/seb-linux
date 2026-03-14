#pragma once

#include "message.h"

#include <QString>
#include <QUuid>

namespace seb::communication::contracts::data {

struct MessageBoxRequestMessage : Message
{
    MessageBoxRequestMessage() = default;
    MessageBoxRequestMessage(int requestedAction, int requestedIcon, QString requestedMessage, QUuid id, QString requestedTitle)
        : action(requestedAction), icon(requestedIcon), message(std::move(requestedMessage)), requestId(id), title(std::move(requestedTitle)) {}

    int action = 0;
    int icon = 0;
    QString message;
    QUuid requestId;
    QString title;
};

}  // namespace seb::communication::contracts::data
