#pragma once

namespace seb::communication::contracts::hosts {

class IHostObject
{
public:
    virtual ~IHostObject() = default;
    virtual void open() = 0;
    virtual void close() = 0;
};

}  // namespace seb::communication::contracts::hosts
