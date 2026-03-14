#include "base_proxy.h"

namespace seb::communication::proxies {

BaseProxy::BaseProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent)
    : QObject(parent)
    , address_(address)
    , factory_(factory)
    , owner_(owner)
{
    pingTimer_.setInterval(60000);
    QObject::connect(&pingTimer_, &QTimer::timeout, this, &BaseProxy::testConnection);
}

bool BaseProxy::isConnected() const
{
    return !communicationToken_.isNull();
}

bool BaseProxy::connect(const QUuid &token, bool autoPing)
{
    proxy_ = factory_.createObject(address_);
    const auto response = proxy_->connect(token);
    communicationToken_ = response.communicationToken;
    if (autoPing && response.connectionEstablished) {
        pingTimer_.start();
    }
    return response.connectionEstablished;
}

bool BaseProxy::disconnect()
{
    if (!proxy_ || communicationToken_.isNull()) {
        return false;
    }

    pingTimer_.stop();
    contracts::data::DisconnectionMessage message;
    message.communicationToken = communicationToken_;
    message.interlocutor = owner_;
    const auto response = proxy_->disconnect(message);
    if (response.connectionTerminated) {
        communicationToken_ = {};
    }
    return response.connectionTerminated;
}

std::unique_ptr<contracts::data::Response> BaseProxy::send(const contracts::data::Message &message)
{
    if (!proxy_ || communicationToken_.isNull()) {
        return {};
    }

    auto copy = message;
    copy.communicationToken = communicationToken_;
    return proxy_->send(copy);
}

std::unique_ptr<contracts::data::Response> BaseProxy::send(contracts::data::SimpleMessagePurport purport)
{
    contracts::data::SimpleMessage message(purport);
    message.communicationToken = communicationToken_;
    return proxy_->send(message);
}

bool BaseProxy::isAcknowledged(const contracts::data::Response *response) const
{
    const auto *simple = dynamic_cast<const contracts::data::SimpleResponse *>(response);
    return simple && simple->purport == contracts::data::SimpleResponsePurport::Acknowledged;
}

void BaseProxy::testConnection()
{
    const auto response = send(contracts::data::SimpleMessagePurport::Ping);
    if (!isAcknowledged(response.get())) {
        pingTimer_.stop();
    }
}

}  // namespace seb::communication::proxies
