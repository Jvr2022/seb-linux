#pragma once

#include "../../settings/AppSettings.h"

#include <QVariantMap>

namespace seb::configuration::configuration_data {

class DataProcessor
{
public:
    void processDefault(seb::settings::AppSettings &settings) const;
    void process(const QVariantMap &data, seb::settings::AppSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data
