#include "proxy_object_factory.h"

namespace seb::communication::proxies {
namespace {

class ProxyObject final : public contracts::proxies::IProxyObject
{
public:
    explicit ProxyObject(QString endpoint)
        : address(std::move(endpoint)) {}

    contracts::data::ConnectionResponse connect(const QUuid &) override { return {}; }
    contracts::data::DisconnectionResponse disconnect(const contracts::data::DisconnectionMessage &) override { return {}; }
    std::unique_ptr<contracts::data::Response> send(const contracts::data::Message &) override { return {}; }

    QString address;
};

}  // namespace

std::unique_ptr<contracts::proxies::IProxyObject> ProxyObjectFactory::createObject(const QString &address)
{
    return std::make_unique<ProxyObject>(address);
}

}  // namespace seb::communication::proxies
