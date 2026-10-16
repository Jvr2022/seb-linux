#pragma once

#include <QUuid>
#include <QUrl>
#include <QString>

namespace seb::browser::contracts {

struct DownloadRequest
{
    QUuid id;
    QUrl url;
    QString suggestedFileName;
    QString mimeType;
    bool accepted = false;
    QString targetDirectory;
    QString targetFileName;
};

struct DownloadResult
{
    QUuid id;
    QUrl url;
    QString localPath;
    bool success = false;
};

}  // namespace seb::browser::contracts
