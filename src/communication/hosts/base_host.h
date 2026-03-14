#pragma once

#include "../contracts/data/simple_message.h"
#include "../contracts/data/simple_response.h"
#include "../contracts/hosts/i_host_object_factory.h"
#include "../contracts/i_communication_host.h"

#include <QSet>

namespace seb::communication::hosts {

class BaseHost : public contracts::ICommunication, public contracts::ICommunicationHost
{
public:
    BaseHost(const QString &address, contracts::hosts::IHostObjectFactory &factory);
    ~BaseHost() override;

    bool isRunning() const override;
    void start() override;
    void stop() override;

    contracts::data::ConnectionResponse connect(const QUuid &token = {}) override;
    contracts::data::DisconnectionResponse disconnect(const contracts::data::DisconnectionMessage &message) override;
    std::unique_ptr<contracts::data::Response> send(const contracts::data::Message &message) override;

protected:
    virtual bool onConnect(const QUuid &token) = 0;
    virtual void onDisconnect(contracts::Interlocutor interlocutor) = 0;
    virtual std::unique_ptr<contracts::data::Response> onReceive(const contracts::data::Message &message) = 0;
    virtual std::unique_ptr<contracts::data::Response> onReceive(contracts::data::SimpleMessagePurport purport) = 0;

private:
    bool isAuthorized(const QUuid &token) const;

    QString address_;
    contracts::hosts::IHostObjectFactory &factory_;
    std::unique_ptr<contracts::hosts::IHostObject> host_;
    QSet<QUuid> communicationTokens_;
};

}  // namespace seb::communication::hosts
