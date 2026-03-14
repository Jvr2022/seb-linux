#pragma once

#include "i_proxy_object.h"

#include <memory>

namespace seb::communication::contracts::proxies {

class IProxyObjectFactory
{
public:
    virtual ~IProxyObjectFactory() = default;
    virtual std::unique_ptr<IProxyObject> createObject(const QString &address) = 0;
};

}  // namespace seb::communication::contracts::proxies
