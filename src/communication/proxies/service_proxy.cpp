#include "service_proxy.h"

#include "../contracts/data/session_start_message.h"
#include "../contracts/data/session_stop_message.h"

namespace seb::communication::proxies {

ServiceProxy::ServiceProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent)
    : BaseProxy(address, factory, owner, parent)
{
}

contracts::proxies::CommunicationResult ServiceProxy::runSystemConfigurationUpdate()
{
    return contracts::proxies::CommunicationResult(isAcknowledged(send(contracts::data::SimpleMessagePurport::UpdateSystemConfiguration).get()));
}

contracts::proxies::CommunicationResult ServiceProxy::startSession(const seb::SebSettings &configuration)
{
    contracts::data::SessionStartMessage message(configuration);
    return contracts::proxies::CommunicationResult(isAcknowledged(send(message).get()));
}

contracts::proxies::CommunicationResult ServiceProxy::stopSession(const QUuid &sessionId)
{
    contracts::data::SessionStopMessage message(sessionId);
    return contracts::proxies::CommunicationResult(isAcknowledged(send(message).get()));
}

}  // namespace seb::communication::proxies
