#pragma once

#include "../load_status.h"

#include <QByteArray>
#include <QUrl>

namespace seb::configuration::contracts::data_resources {

class IResourceLoader
{
public:
    virtual ~IResourceLoader() = default;
    virtual bool canLoad(const QUrl &resource) const = 0;
    virtual LoadStatus tryLoad(const QUrl &resource, QByteArray &data) const = 0;
};

}  // namespace seb::configuration::contracts::data_resources
