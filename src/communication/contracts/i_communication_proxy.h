#pragma once

#include "events/communication_event_handler.h"

#include <QUuid>

namespace seb::communication::contracts {

class ICommunicationProxy
{
public:
    virtual ~ICommunicationProxy() = default;

    virtual bool isConnected() const = 0;
    virtual bool connect(const QUuid &token = {}, bool autoPing = true) = 0;
    virtual bool disconnect() = 0;
};

}  // namespace seb::communication::contracts
