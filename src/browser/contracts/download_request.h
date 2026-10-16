#pragma once

#include <QUrl>
#include <QString>

namespace seb::browser::contracts {

struct DownloadRequest
{
    QUrl url;
    QString suggestedFileName;
    QString mimeType;
    bool accepted = false;
    QString targetDirectory;
    QString targetFileName;
};

}  // namespace seb::browser::contracts
