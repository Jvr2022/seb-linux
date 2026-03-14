#pragma once

#include "contracts/coordinator.h"

#include <QMutex>

namespace seb::client {

class Coordinator : public contracts::ICoordinator
{
public:
    bool isReconfigurationLocked() const override;
    bool isSessionLocked() const override;
    void releaseReconfigurationLock() override;
    void releaseSessionLock() override;
    bool requestReconfigurationLock() override;
    bool requestSessionLock() override;

private:
    mutable QMutex mutex_;
    bool reconfigurationLocked_ = false;
    bool sessionLocked_ = false;
};

}  // namespace seb::client
