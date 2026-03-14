#pragma once

namespace seb::communication::contracts::data {

enum class SimpleMessagePurport
{
    Authenticate = 1,
    ClientIsReady,
    ConfigurationNeeded,
    Ping,
    ReconfigurationAborted,
    RequestShutdown,
    Shutdown,
    UpdateSystemConfiguration
};

}  // namespace seb::communication::contracts::data
