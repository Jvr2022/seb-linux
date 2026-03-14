#pragma once

#include "../contracts/hosts/i_host_object_factory.h"

namespace seb::communication::hosts {

class HostObjectFactory : public contracts::hosts::IHostObjectFactory
{
public:
    std::unique_ptr<contracts::hosts::IHostObject> createObject(const QString &address, contracts::ICommunication *communicationObject) override;
};

}  // namespace seb::communication::hosts
