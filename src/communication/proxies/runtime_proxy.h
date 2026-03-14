#pragma once

#include "base_proxy.h"
#include "../contracts/proxies/i_runtime_proxy.h"

namespace seb::communication::proxies {

class RuntimeProxy : public BaseProxy, public contracts::proxies::IRuntimeProxy
{
    Q_OBJECT

public:
    RuntimeProxy(const QString &address, contracts::proxies::IProxyObjectFactory &factory, contracts::Interlocutor owner, QObject *parent = nullptr);

    bool isConnected() const override { return BaseProxy::isConnected(); }
    bool connect(const QUuid &token = {}, bool autoPing = true) override { return BaseProxy::connect(token, autoPing); }
    bool disconnect() override { return BaseProxy::disconnect(); }

    contracts::proxies::TypedCommunicationResult<contracts::data::ConfigurationResponse> getConfiguration() override;
    contracts::proxies::CommunicationResult informClientReady() override;
    contracts::proxies::CommunicationResult requestShutdown() override;
    contracts::proxies::CommunicationResult requestReconfiguration(const QString &filePath, const QString &url) override;
};

}  // namespace seb::communication::proxies
