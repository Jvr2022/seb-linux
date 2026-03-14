#include "host_object_factory.h"

namespace seb::communication::hosts {
namespace {

class HostObject final : public contracts::hosts::IHostObject
{
public:
    explicit HostObject(QString endpoint)
        : address(std::move(endpoint)) {}

    void open() override { opened = true; }
    void close() override { opened = false; }

    QString address;
    bool opened = false;
};

}  // namespace

std::unique_ptr<contracts::hosts::IHostObject> HostObjectFactory::createObject(const QString &address, contracts::ICommunication *)
{
    return std::make_unique<HostObject>(address);
}

}  // namespace seb::communication::hosts
