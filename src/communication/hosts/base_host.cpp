#include "base_host.h"

namespace seb::communication::hosts {

BaseHost::BaseHost(const QString &address, contracts::hosts::IHostObjectFactory &factory)
    : address_(address)
    , factory_(factory)
{
}

BaseHost::~BaseHost() = default;

bool BaseHost::isRunning() const
{
    return host_ != nullptr;
}

void BaseHost::start()
{
    if (!host_) {
        host_ = factory_.createObject(address_, this);
    }
    if (host_) {
        host_->open();
    }
}

void BaseHost::stop()
{
    if (host_) {
        host_->close();
        host_.reset();
    }
}

contracts::data::ConnectionResponse BaseHost::connect(const QUuid &token)
{
    contracts::data::ConnectionResponse response;
    response.connectionEstablished = onConnect(token);
    if (response.connectionEstablished) {
        response.communicationToken = QUuid::createUuid();
        communicationTokens_.insert(response.communicationToken);
    }
    return response;
}

contracts::data::DisconnectionResponse BaseHost::disconnect(const contracts::data::DisconnectionMessage &message)
{
    contracts::data::DisconnectionResponse response;
    if (isAuthorized(message.communicationToken)) {
        onDisconnect(message.interlocutor);
        communicationTokens_.remove(message.communicationToken);
        response.connectionTerminated = true;
    }
    return response;
}

std::unique_ptr<contracts::data::Response> BaseHost::send(const contracts::data::Message &message)
{
    if (!isAuthorized(message.communicationToken)) {
        return std::make_unique<contracts::data::SimpleResponse>(contracts::data::SimpleResponsePurport::Unauthorized);
    }

    if (const auto *simpleMessage = dynamic_cast<const contracts::data::SimpleMessage *>(&message)) {
        if (simpleMessage->purport == contracts::data::SimpleMessagePurport::Ping) {
            return std::make_unique<contracts::data::SimpleResponse>(contracts::data::SimpleResponsePurport::Acknowledged);
        }
        return onReceive(simpleMessage->purport);
    }

    return onReceive(message);
}

bool BaseHost::isAuthorized(const QUuid &token) const
{
    return !token.isNull() && communicationTokens_.contains(token);
}

}  // namespace seb::communication::hosts
