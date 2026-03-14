#pragma once

#include "../../../seb_settings.h"

#include <QVariantMap>

namespace seb::configuration::configuration_data::data_mapping {

class BaseDataMapper
{
public:
    virtual ~BaseDataMapper() = default;
    virtual void map(const QVariantMap &data, seb::SebSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data::data_mapping
