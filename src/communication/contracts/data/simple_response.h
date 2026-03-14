#pragma once

#include "response.h"
#include "simple_response_purport.h"

namespace seb::communication::contracts::data {

struct SimpleResponse : Response
{
    explicit SimpleResponse(SimpleResponsePurport value)
        : purport(value) {}

    SimpleResponsePurport purport;
};

}  // namespace seb::communication::contracts::data
