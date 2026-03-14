#include "coordinator.h"

#include <QMutexLocker>

namespace seb::client {

bool Coordinator::isReconfigurationLocked() const
{
    QMutexLocker locker(&mutex_);
    return reconfigurationLocked_;
}

bool Coordinator::isSessionLocked() const
{
    QMutexLocker locker(&mutex_);
    return sessionLocked_;
}

void Coordinator::releaseReconfigurationLock()
{
    QMutexLocker locker(&mutex_);
    reconfigurationLocked_ = false;
}

void Coordinator::releaseSessionLock()
{
    QMutexLocker locker(&mutex_);
    sessionLocked_ = false;
}

bool Coordinator::requestReconfigurationLock()
{
    QMutexLocker locker(&mutex_);
    if (reconfigurationLocked_) {
        return false;
    }
    reconfigurationLocked_ = true;
    return true;
}

bool Coordinator::requestSessionLock()
{
    QMutexLocker locker(&mutex_);
    if (sessionLocked_) {
        return false;
    }
    sessionLocked_ = true;
    return true;
}

}  // namespace seb::client
