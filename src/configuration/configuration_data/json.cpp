#include "json.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QDateTime>
#include <QMetaType>

#include <algorithm>

namespace {

using namespace seb::configuration::configuration_data;

bool isOriginatorVersion(const QString &key)
{
    return key.compare(QStringLiteral("originatorVersion"), Qt::CaseInsensitive) == 0;
}

void appendValue(QByteArray &output, const QVariant &value);

bool hasSerializableEntries(const QVariantMap &map)
{
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (isOriginatorVersion(it.key())) continue;
        if (it.value().metaType().id() == QMetaType::QVariantMap &&
            !hasSerializableEntries(it.value().toMap())) {
            continue;
        }
        return true;
    }
    return false;
}

void appendString(QByteArray &output, const QString &value)
{
    output.append('\"');
    output.append(value.toUtf8());
    output.append('\"');
}

void appendDateTime(QByteArray &output, const QDateTime &value)
{
    QString text = value.toString(Qt::ISODateWithMs);
    if (!value.isValid()) {
        appendString(output, QString());
        return;
    }

    const int decimal = text.indexOf(QLatin1Char('.'));
    if (decimal >= 0) {
        int end = decimal + 1;
        while (end < text.size() && text.at(end).isDigit()) {
            ++end;
        }
        while (end - decimal - 1 < 7) {
            text.insert(end, QLatin1Char('0'));
            ++end;
        }
    } else {
        const int zone = text.indexOf(QLatin1Char('Z'), 19);
        const int offset = text.indexOf(QLatin1Char('+'), 19);
        const int negativeOffset = text.indexOf(QLatin1Char('-'), 19);
        int insertAt = text.size();
        if (zone >= 0) insertAt = zone;
        if (offset >= 0) insertAt = std::min(insertAt, offset);
        if (negativeOffset >= 0) insertAt = std::min(insertAt, negativeOffset);
        text.insert(insertAt, QStringLiteral(".0000000"));
    }

    output.append(text.toUtf8());
}

void appendMap(QByteArray &output, const QVariantMap &map)
{
    QList<QString> keys;
    keys.reserve(map.size());
    for (auto it = map.cbegin(); it != map.cend(); ++it) {
        if (isOriginatorVersion(it.key())) continue;
        if (it.value().metaType().id() == QMetaType::QVariantMap &&
            !hasSerializableEntries(it.value().toMap())) continue;
        keys.push_back(it.key());
    }
    std::sort(keys.begin(), keys.end(), [](const QString &left, const QString &right) {
        return QString::compare(left, right, Qt::CaseSensitive) < 0;
    });

    output.append('{');
    for (qsizetype index = 0; index < keys.size(); ++index) {
        if (index > 0) output.append(',');
        const QString &key = keys.at(index);
        appendString(output, key);
        output.append(':');
        appendValue(output, map.value(key));
    }
    output.append('}');
}

void appendList(QByteArray &output, const QVariantList &list)
{
    output.append('[');
    for (qsizetype index = 0; index < list.size(); ++index) {
        if (index > 0) output.append(',');
        appendValue(output, list.at(index));
    }
    output.append(']');
}

void appendValue(QByteArray &output, const QVariant &value)
{
    switch (value.metaType().id()) {
    case QMetaType::QVariantMap:
        appendMap(output, value.toMap());
        return;
    case QMetaType::QVariantList:
        appendList(output, value.toList());
        return;
    case QMetaType::QByteArray:
        appendString(output, QString::fromLatin1(value.toByteArray().toBase64()));
        return;
    case QMetaType::QDateTime:
        appendDateTime(output, value.toDateTime());
        return;
    case QMetaType::Bool:
        output.append(value.toBool() ? "true" : "false");
        return;
    case QMetaType::Int:
        output.append(QByteArray::number(value.toInt()));
        return;
    case QMetaType::LongLong:
        output.append(QByteArray::number(value.toLongLong()));
        return;
    case QMetaType::UInt:
        output.append(QByteArray::number(value.toUInt()));
        return;
    case QMetaType::ULongLong:
        output.append(QByteArray::number(value.toULongLong()));
        return;
    case QMetaType::Double:
        output.append(QString::number(value.toDouble(), 'g', 15).toLatin1());
        return;
    case QMetaType::QString:
        appendString(output, value.toString());
        return;
    default:
        appendString(output, QString());
        return;
    }
}

}  // namespace

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

QByteArray Json::serialize(const QVariantMap &data)
{
    QByteArray result;
    appendMap(result, data);
    return result;
}

}  // namespace seb::configuration::configuration_data
