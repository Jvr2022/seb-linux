#include "client_proxy.h"

#include "../contracts/data/authentication_response.h"
#include "../contracts/data/reconfiguration_denied_message.h"

namespace seb::communication::proxies {

ClientProxy::ClientProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent)
    : BaseProxy(address, factory, owner, parent)
{
}

contracts::proxies::CommunicationResult ClientProxy::informReconfigurationAborted()
{
    return contracts::proxies::CommunicationResult(isAcknowledged(send(contracts::data::SimpleMessagePurport::ReconfigurationAborted).get()));
}

contracts::proxies::CommunicationResult ClientProxy::informReconfigurationDenied(const QString &filePath)
{
    contracts::data::ReconfigurationDeniedMessage message(filePath);
    return contracts::proxies::CommunicationResult(isAcknowledged(send(message).get()));
}

contracts::proxies::CommunicationResult ClientProxy::initiateShutdown()
{
    return contracts::proxies::CommunicationResult(isAcknowledged(send(contracts::data::SimpleMessagePurport::Shutdown).get()));
}

contracts::proxies::TypedCommunicationResult<contracts::data::AuthenticationResponse> ClientProxy::requestAuthentication()
{
    auto response = send(contracts::data::SimpleMessagePurport::Authenticate);
    if (auto *typed = dynamic_cast<contracts::data::AuthenticationResponse *>(response.get())) {
        return contracts::proxies::TypedCommunicationResult<contracts::data::AuthenticationResponse>(true, *typed);
    }
    return contracts::proxies::TypedCommunicationResult<contracts::data::AuthenticationResponse>(false, {});
}

}  // namespace seb::communication::proxies
