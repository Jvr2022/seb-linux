#pragma once

#include "download_event_args.h"

#include <QString>
#include <functional>

namespace seb::browser::contracts::events {

using DownloadRequestedEventHandler = std::function<void(const QString &fileName, DownloadEventArgs &)>;

}  // namespace seb::browser::contracts::events
