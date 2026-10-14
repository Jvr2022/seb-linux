#pragma once

#include "../../settings/AppSettings.h"

#include <QVariantMap>

#include <memory>

namespace seb::configuration::configuration_data {

class DataMapper
{
public:
    DataMapper();
    ~DataMapper();
    void map(const QVariantMap &data, seb::settings::AppSettings &settings) const;

private:
    class MapperCollection;
    std::unique_ptr<MapperCollection> mappers_;
};

}  // namespace seb::configuration::configuration_data
