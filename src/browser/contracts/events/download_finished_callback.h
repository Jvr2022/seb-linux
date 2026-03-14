#pragma once

#include <QString>
#include <functional>

namespace seb::browser::contracts::events {

using DownloadFinishedCallback = std::function<void(bool success, const QString &url, const QString &filePath)>;

}  // namespace seb::browser::contracts::events
