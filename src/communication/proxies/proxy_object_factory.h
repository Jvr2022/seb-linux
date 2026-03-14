#pragma once

#include "../contracts/proxies/i_proxy_object_factory.h"

namespace seb::communication::proxies {

class ProxyObjectFactory : public contracts::proxies::IProxyObjectFactory
{
public:
    std::unique_ptr<contracts::proxies::IProxyObject> createObject(const QString &address) override;
};

}  // namespace seb::communication::proxies
