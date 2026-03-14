#pragma once

#include "response.h"

namespace seb::communication::contracts::data {

struct DisconnectionResponse : Response
{
    bool connectionTerminated = false;
};

}  // namespace seb::communication::contracts::data
