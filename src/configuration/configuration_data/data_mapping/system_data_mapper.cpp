#include "system_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void SystemDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key == keys::System::AlwaysOnTop) settings.System.AlwaysOn = helpers::boolean(value, settings.System.AlwaysOn);
    else if (key == keys::System::ShowApplicationChooser) settings.System.ShowApplicationChooser = helpers::boolean(value, settings.System.ShowApplicationChooser);
}
}  // namespace seb::configuration::configuration_data::data_mapping
