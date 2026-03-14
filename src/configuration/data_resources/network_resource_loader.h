#pragma once

#include "../contracts/app_config.h"
#include "../contracts/data_resources/i_resource_loader.h"

namespace seb::configuration::data_resources {

class NetworkResourceLoader : public contracts::data_resources::IResourceLoader
{
public:
    explicit NetworkResourceLoader(const contracts::AppConfig &appConfig);

    bool canLoad(const QUrl &resource) const override;
    contracts::LoadStatus tryLoad(const QUrl &resource, QByteArray &data) const override;

private:
    contracts::AppConfig appConfig_;
};

}  // namespace seb::configuration::data_resources
