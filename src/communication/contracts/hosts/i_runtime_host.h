#pragma once

#include "../i_communication_host.h"

namespace seb::communication::contracts::hosts {

class IRuntimeHost : public contracts::ICommunicationHost
{
public:
    ~IRuntimeHost() override = default;
    virtual void setAllowConnection(bool allow) = 0;
    virtual void setAuthenticationToken(const QUuid &token) = 0;
};

}  // namespace seb::communication::contracts::hosts
