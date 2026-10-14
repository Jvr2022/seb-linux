#include "display_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void DisplayDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key == keys::Display::AllowedDisplays) settings.Display.AllowedDisplays = helpers::integer(value, settings.Display.AllowedDisplays);
    else if (key == keys::Display::AlwaysOn) settings.Display.AlwaysOn = helpers::boolean(value, settings.Display.AlwaysOn);
    else if (key == keys::Display::IgnoreError) settings.Display.IgnoreError = helpers::boolean(value, settings.Display.IgnoreError);
    else if (key == keys::Display::InternalDisplayOnly) settings.Display.InternalDisplayOnly = helpers::boolean(value, settings.Display.InternalDisplayOnly);
    else if (key == QStringLiteral("allowMultipleDisplays")) settings.Display.DisableMultipleDisplays = !helpers::boolean(value, true);
    else if (key == QStringLiteral("enforcePrimaryDisplay")) settings.Display.EnforcePrimaryDisplay = helpers::boolean(value);
}
}  // namespace seb::configuration::configuration_data::data_mapping
