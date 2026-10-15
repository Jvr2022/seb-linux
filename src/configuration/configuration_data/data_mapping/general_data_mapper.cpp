#include "general_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void GeneralDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key != keys::General::LogLevel || !helpers::isInteger(value)) return;

    switch (helpers::integer(value)) {
    case 0: settings.LogLevel = seb::settings::logging::LogLevel::Error; break;
    case 1: settings.LogLevel = seb::settings::logging::LogLevel::Warning; break;
    case 2: settings.LogLevel = seb::settings::logging::LogLevel::Info; break;
    default: settings.LogLevel = seb::settings::logging::LogLevel::Debug; break;
    }
}
}  // namespace seb::configuration::configuration_data::data_mapping
