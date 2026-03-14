#pragma once

#include "data/connection_response.h"
#include "data/disconnection_message.h"
#include "data/disconnection_response.h"
#include "data/message.h"
#include "data/response.h"

#include <memory>

namespace seb::communication::contracts {

class ICommunication
{
public:
    virtual ~ICommunication() = default;

    virtual data::ConnectionResponse connect(const QUuid &token = {}) = 0;
    virtual data::DisconnectionResponse disconnect(const data::DisconnectionMessage &message) = 0;
    virtual std::unique_ptr<data::Response> send(const data::Message &message) = 0;
};

}  // namespace seb::communication::contracts
