#pragma once

namespace seb::communication::contracts::data {

enum class SimpleResponsePurport
{
    Acknowledged = 1,
    Unauthorized,
    UnknownMessage
};

}  // namespace seb::communication::contracts::data
