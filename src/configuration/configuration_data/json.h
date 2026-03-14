#pragma once

#include <QJsonObject>
#include <QVariantMap>

namespace seb::configuration::configuration_data {

QVariantMap jsonToVariantMap(const QJsonObject &object);
QJsonObject variantMapToJson(const QVariantMap &map);

}  // namespace seb::configuration::configuration_data
