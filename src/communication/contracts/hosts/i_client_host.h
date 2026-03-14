#pragma once

#include "../events/communication_event_handler.h"
#include "../i_communication_host.h"

namespace seb::communication::contracts::hosts {

class IClientHost : public contracts::ICommunicationHost
{
public:
    ~IClientHost() override = default;
    virtual void setAuthenticationToken(const QUuid &token) = 0;
    virtual bool isConnected() const = 0;
};

}  // namespace seb::communication::contracts::hosts
