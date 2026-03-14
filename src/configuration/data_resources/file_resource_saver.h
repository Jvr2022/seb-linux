#pragma once

#include "../contracts/data_resources/i_resource_saver.h"

namespace seb::configuration::data_resources {

class FileResourceSaver : public contracts::data_resources::IResourceSaver
{
public:
    bool canSave(const QUrl &destination) const override;
    contracts::SaveStatus trySave(const QUrl &destination, const QByteArray &data) const override;
};

}  // namespace seb::configuration::data_resources
