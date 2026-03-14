#pragma once

#include "../data/configuration_response.h"
#include "../i_communication_proxy.h"
#include "communication_result.h"

namespace seb::communication::contracts::proxies {

class IRuntimeProxy : public contracts::ICommunicationProxy
{
public:
    ~IRuntimeProxy() override = default;

    virtual TypedCommunicationResult<data::ConfigurationResponse> getConfiguration() = 0;
    virtual CommunicationResult informClientReady() = 0;
    virtual CommunicationResult requestShutdown() = 0;
    virtual CommunicationResult requestReconfiguration(const QString &filePath, const QString &url) = 0;
};

}  // namespace seb::communication::contracts::proxies
