#include "data_mapper.h"

#include "keys.h"
#include "../../seb_settings.h"

namespace seb::configuration::configuration_data {

void DataMapper::map(const QVariantMap &data, seb::SebSettings &settings) const
{
    const QByteArray raw = data.value(keys::Meta::RawSebData).toByteArray();
    if (raw.isEmpty()) {
        return;
    }

    const seb::LoadResult result = seb::loadSettingsFromData(raw);
    if (result.ok) {
        settings = result.settings;
    }
}

}  // namespace seb::configuration::configuration_data
