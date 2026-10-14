#include "general_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void GeneralDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key == keys::General::LogLevel) settings.LogLevel = static_cast<seb::settings::logging::LogLevel>(helpers::integer(value, static_cast<int>(settings.LogLevel)));
}
}  // namespace seb::configuration::configuration_data::data_mapping
