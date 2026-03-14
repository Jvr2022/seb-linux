#pragma once

#include "../data/authentication_response.h"
#include "../data/password_request_purpose.h"
#include "../i_communication_proxy.h"
#include "communication_result.h"

namespace seb::communication::contracts::proxies {

class IClientProxy : public contracts::ICommunicationProxy
{
public:
    ~IClientProxy() override = default;

    virtual CommunicationResult informReconfigurationAborted() = 0;
    virtual CommunicationResult informReconfigurationDenied(const QString &filePath) = 0;
    virtual CommunicationResult initiateShutdown() = 0;
    virtual TypedCommunicationResult<data::AuthenticationResponse> requestAuthentication() = 0;
};

}  // namespace seb::communication::contracts::proxies
