#pragma once

#include "download_finished_callback.h"

#include <QString>

namespace seb::browser::contracts::events {

struct DownloadEventArgs
{
    bool allowDownload = false;
    DownloadFinishedCallback callback;
    QString downloadPath;
    QString url;
};

}  // namespace seb::browser::contracts::events
