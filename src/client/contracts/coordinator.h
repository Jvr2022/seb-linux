#pragma once

namespace seb::client::contracts {

class ICoordinator
{
public:
    virtual ~ICoordinator() = default;

    virtual bool isReconfigurationLocked() const = 0;
    virtual bool isSessionLocked() const = 0;
    virtual void releaseReconfigurationLock() = 0;
    virtual void releaseSessionLock() = 0;
    virtual bool requestReconfigurationLock() = 0;
    virtual bool requestSessionLock() = 0;
};

}  // namespace seb::client::contracts
