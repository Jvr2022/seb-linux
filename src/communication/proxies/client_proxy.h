#pragma once

#include "base_proxy.h"
#include "../contracts/proxies/i_client_proxy.h"

namespace seb::communication::proxies {

class ClientProxy : public BaseProxy, public contracts::proxies::IClientProxy
{
    Q_OBJECT

public:
    ClientProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent = nullptr);

    bool isConnected() const override { return BaseProxy::isConnected(); }
    bool connect(const QUuid &token = {}, bool autoPing = true) override { return BaseProxy::connect(token, autoPing); }
    bool disconnect() override { return BaseProxy::disconnect(); }

    contracts::proxies::CommunicationResult informReconfigurationAborted() override;
    contracts::proxies::CommunicationResult informReconfigurationDenied(const QString &filePath) override;
    contracts::proxies::CommunicationResult initiateShutdown() override;
    contracts::proxies::TypedCommunicationResult<contracts::data::AuthenticationResponse> requestAuthentication() override;
};

}  // namespace seb::communication::proxies
