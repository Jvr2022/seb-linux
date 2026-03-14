#pragma once

#include "response.h"

namespace seb::communication::contracts::data {

struct AuthenticationResponse : Response
{
    int processId = 0;
};

}  // namespace seb::communication::contracts::data
