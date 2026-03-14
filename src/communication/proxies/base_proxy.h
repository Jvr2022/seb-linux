#pragma once

#include "../contracts/data/disconnection_message.h"
#include "../contracts/data/simple_message.h"
#include "../contracts/data/simple_response.h"
#include "../contracts/i_communication_proxy.h"
#include "../contracts/proxies/i_proxy_object_factory.h"

#include <QObject>
#include <QTimer>

namespace seb::communication::proxies {

class BaseProxy : public QObject, public contracts::ICommunicationProxy
{
    Q_OBJECT

public:
    BaseProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent = nullptr);

    bool isConnected() const override;
    bool connect(const QUuid &token = {}, bool autoPing = true) override;
    bool disconnect() override;

protected:
    std::unique_ptr<contracts::data::Response> send(const contracts::data::Message &message);
    std::unique_ptr<contracts::data::Response> send(contracts::data::SimpleMessagePurport purport);
    bool isAcknowledged(const contracts::data::Response *response) const;

private slots:
    void testConnection();

private:
    QString address_;
    contracts::proxies::IProxyObjectFactory &factory_;
    contracts::Interlocutor owner_;
    std::unique_ptr<contracts::proxies::IProxyObject> proxy_;
    QUuid communicationToken_;
    QTimer pingTimer_;
};

}  // namespace seb::communication::proxies
