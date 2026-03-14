#pragma once

#include "../contracts/proxies/i_proxy_factory.h"
#include "../contracts/proxies/i_proxy_object_factory.h"

namespace seb::communication::proxies {

class ProxyFactory : public contracts::proxies::IProxyFactory
{
public:
    explicit ProxyFactory(contracts::proxies::IProxyObjectFactory &factory);

    std::unique_ptr<contracts::proxies::IClientProxy> createClientProxy(const QString &address, contracts::Interlocutor owner) override;

private:
    contracts::proxies::IProxyObjectFactory &factory_;
};

}  // namespace seb::communication::proxies
