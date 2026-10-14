#pragma once

#include "../../../settings/AppSettings.h"

#include <QString>
#include <QVariant>
#include <QVariantMap>

namespace seb::configuration::configuration_data::data_mapping {

class BaseDataMapper
{
public:
    virtual ~BaseDataMapper() = default;
    virtual void map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const = 0;
    virtual void mapGlobal(const QVariantMap &data, seb::settings::AppSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data::data_mapping
