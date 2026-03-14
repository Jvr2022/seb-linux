#include "proxy_factory.h"

#include "client_proxy.h"

namespace seb::communication::proxies {

ProxyFactory::ProxyFactory(contracts::proxies::IProxyObjectFactory &factory)
    : factory_(factory)
{
}

std::unique_ptr<contracts::proxies::IClientProxy> ProxyFactory::createClientProxy(const QString &address, contracts::Interlocutor owner)
{
    return std::make_unique<ClientProxy>(address, factory_, owner);
}

}  // namespace seb::communication::proxies
