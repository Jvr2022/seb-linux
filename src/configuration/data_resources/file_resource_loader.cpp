#include "file_resource_loader.h"

#include <QFile>

namespace seb::configuration::data_resources {

bool FileResourceLoader::canLoad(const QUrl &resource) const
{
    return resource.isLocalFile() || resource.scheme() == QStringLiteral("file");
}

contracts::LoadStatus FileResourceLoader::tryLoad(const QUrl &resource, QByteArray &data) const
{
    QFile file(resource.toLocalFile());
    if (!file.open(QIODevice::ReadOnly)) {
        return contracts::LoadStatus::NotSupported;
    }
    data = file.readAll();
    return contracts::LoadStatus::Success;
}

}  // namespace seb::configuration::data_resources
