#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <QVariantMap>

namespace seb::configuration::configuration_data {

class Json
{
public:
    static QByteArray serialize(const QVariantMap &data);
};

QVariantMap jsonToVariantMap(const QJsonObject &object);
QJsonObject variantMapToJson(const QVariantMap &map);

}  // namespace seb::configuration::configuration_data
