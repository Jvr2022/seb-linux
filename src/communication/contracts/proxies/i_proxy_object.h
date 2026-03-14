#pragma once

#include "../i_communication.h"

namespace seb::communication::contracts::proxies {

class IProxyObject : public contracts::ICommunication
{
public:
    ~IProxyObject() override = default;
};

}  // namespace seb::communication::contracts::proxies
