#include "json.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QMetaType>

namespace seb::configuration::configuration_data {
namespace {

QVariant toVariant(const QJsonValue &value)
{
    if (value.isObject()) {
        return jsonToVariantMap(value.toObject());
    }
    if (value.isArray()) {
        QVariantList list;
        const QJsonArray array = value.toArray();
        list.reserve(array.size());
        for (const QJsonValue &item : array) {
            list.push_back(toVariant(item));
        }
        return list;
    }
    return value.toVariant();
}

QJsonValue toJson(const QVariant &value)
{
    if (value.metaType().id() == QMetaType::QVariantMap) {
        return variantMapToJson(value.toMap());
    }
    if (value.metaType().id() == QMetaType::QVariantList) {
        QJsonArray array;
        for (const QVariant &item : value.toList()) {
            array.push_back(toJson(item));
        }
        return array;
    }
    return QJsonValue::fromVariant(value);
}

}  // namespace

QVariantMap jsonToVariantMap(const QJsonObject &object)
{
    QVariantMap map;
    for (auto it = object.begin(); it != object.end(); ++it) {
        map.insert(it.key(), toVariant(it.value()));
    }
    return map;
}

QJsonObject variantMapToJson(const QVariantMap &map)
{
    QJsonObject object;
    for (auto it = map.begin(); it != map.end(); ++it) {
        object.insert(it.key(), toJson(it.value()));
    }
    return object;
}

}  // namespace seb::configuration::configuration_data
