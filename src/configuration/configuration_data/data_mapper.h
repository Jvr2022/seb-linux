#pragma once

#include "../../seb_settings.h"

#include <QVariantMap>

namespace seb::configuration::configuration_data {

class DataMapper
{
public:
    void map(const QVariantMap &data, seb::SebSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data
