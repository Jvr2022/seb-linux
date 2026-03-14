#include "file_resource_saver.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace seb::configuration::data_resources {

bool FileResourceSaver::canSave(const QUrl &destination) const
{
    return destination.isLocalFile() || destination.scheme() == QStringLiteral("file");
}

contracts::SaveStatus FileResourceSaver::trySave(const QUrl &destination, const QByteArray &data) const
{
    const QString path = destination.toLocalFile();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return contracts::SaveStatus::UnexpectedError;
    }
    file.write(data);
    return contracts::SaveStatus::Success;
}

}  // namespace seb::configuration::data_resources
