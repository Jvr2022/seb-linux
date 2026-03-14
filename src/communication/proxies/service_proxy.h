#pragma once

#include "base_proxy.h"
#include "../contracts/proxies/i_service_proxy.h"

namespace seb::communication::proxies {

class ServiceProxy : public BaseProxy, public contracts::proxies::IServiceProxy
{
    Q_OBJECT

public:
    ServiceProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent = nullptr);

    bool isConnected() const override { return BaseProxy::isConnected(); }
    bool connect(const QUuid &token = {}, bool autoPing = true) override { return BaseProxy::connect(token, autoPing); }
    bool disconnect() override { return BaseProxy::disconnect(); }

    contracts::proxies::CommunicationResult runSystemConfigurationUpdate() override;
    contracts::proxies::CommunicationResult startSession(const seb::SebSettings &configuration) override;
    contracts::proxies::CommunicationResult stopSession(const QUuid &sessionId) override;
};

}  // namespace seb::communication::proxies
