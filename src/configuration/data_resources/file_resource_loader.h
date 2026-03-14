#pragma once

#include "../contracts/data_resources/i_resource_loader.h"

namespace seb::configuration::data_resources {

class FileResourceLoader : public contracts::data_resources::IResourceLoader
{
public:
    bool canLoad(const QUrl &resource) const override;
    contracts::LoadStatus tryLoad(const QUrl &resource, QByteArray &data) const override;
};

}  // namespace seb::configuration::data_resources
