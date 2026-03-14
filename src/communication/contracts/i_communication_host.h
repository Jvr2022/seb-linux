#pragma once

namespace seb::communication::contracts {

class ICommunicationHost
{
public:
    virtual ~ICommunicationHost() = default;

    virtual bool isRunning() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

}  // namespace seb::communication::contracts
