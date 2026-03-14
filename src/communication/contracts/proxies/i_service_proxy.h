#pragma once

#include "../i_communication_proxy.h"
#include "communication_result.h"
#include "../../../seb_settings.h"

namespace seb::communication::contracts::proxies {

class IServiceProxy : public contracts::ICommunicationProxy
{
public:
    ~IServiceProxy() override = default;

    virtual CommunicationResult runSystemConfigurationUpdate() = 0;
    virtual CommunicationResult startSession(const seb::SebSettings &configuration) = 0;
    virtual CommunicationResult stopSession(const QUuid &sessionId) = 0;
};

}  // namespace seb::communication::contracts::proxies
