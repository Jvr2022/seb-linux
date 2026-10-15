#include "configuration_file_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ConfigurationFileDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key == keys::ConfigurationFile::ConfigurationPurpose && helpers::isInteger(value)) settings.ConfigurationMode = helpers::integer(value) == 1 ? seb::settings::ConfigurationMode::ConfigureClient : seb::settings::ConfigurationMode::Exam;
    else if (key == keys::ConfigurationFile::SessionMode && helpers::isInteger(value)) settings.SessionMode = helpers::integer(value) == 1 ? seb::settings::SessionMode::Server : seb::settings::SessionMode::Normal;
}
}  // namespace seb::configuration::configuration_data::data_mapping
