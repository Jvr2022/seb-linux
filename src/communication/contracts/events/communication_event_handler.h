#pragma once

#include "communication_event_args.h"

#include <functional>

namespace seb::communication::contracts::events {

using CommunicationEventHandler = std::function<void()>;
template <typename T>
using TypedCommunicationEventHandler = std::function<void(const T &)>;

}  // namespace seb::communication::contracts::events
