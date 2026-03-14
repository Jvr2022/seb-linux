#pragma once

#include "../../seb_settings.h"

#include <QVariantMap>

namespace seb::configuration::configuration_data {

class DataProcessor
{
public:
    void processDefault(seb::SebSettings &settings) const;
    void process(const QVariantMap &data, seb::SebSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data
