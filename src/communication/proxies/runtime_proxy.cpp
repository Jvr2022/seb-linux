#include "runtime_proxy.h"

#include "../contracts/data/configuration_response.h"
#include "../contracts/data/reconfiguration_message.h"

namespace seb::communication::proxies {

RuntimeProxy::RuntimeProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent)
    : BaseProxy(address, factory, owner, parent)
{
}

contracts::proxies::TypedCommunicationResult<contracts::data::ConfigurationResponse> RuntimeProxy::getConfiguration()
{
    auto response = send(contracts::data::SimpleMessagePurport::ConfigurationNeeded);
    if (auto *typed = dynamic_cast<contracts::data::ConfigurationResponse *>(response.get())) {
        return contracts::proxies::TypedCommunicationResult<contracts::data::ConfigurationResponse>(true, *typed);
    }
    return contracts::proxies::TypedCommunicationResult<contracts::data::ConfigurationResponse>(false, {});
}

contracts::proxies::CommunicationResult RuntimeProxy::informClientReady()
{
    return contracts::proxies::CommunicationResult(isAcknowledged(send(contracts::data::SimpleMessagePurport::ClientIsReady).get()));
}

contracts::proxies::CommunicationResult RuntimeProxy::requestShutdown()
{
    return contracts::proxies::CommunicationResult(isAcknowledged(send(contracts::data::SimpleMessagePurport::RequestShutdown).get()));
}

contracts::proxies::CommunicationResult RuntimeProxy::requestReconfiguration(const QString &filePath, const QString &url)
{
    contracts::data::ReconfigurationMessage message(filePath, url);
    return contracts::proxies::CommunicationResult(isAcknowledged(send(message).get()));
}

}  // namespace seb::communication::proxies
