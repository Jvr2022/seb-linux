#pragma once

#include <QMetaType>
#include <QRegularExpression>
#include <QStringList>
#include <QVariant>

namespace seb::configuration::configuration_data::data_mapping::helpers {

inline bool boolean(const QVariant &value, bool fallback = false)
{
    return value.metaType().id() == QMetaType::Bool ? value.toBool() : fallback;
}

inline int integer(const QVariant &value, int fallback = 0)
{
    return value.metaType().id() == QMetaType::Int ? value.toInt() : fallback;
}

inline double real(const QVariant &value, double fallback = 0.0)
{
    return value.metaType().id() == QMetaType::Double ? value.toDouble() : fallback;
}

inline bool isBoolean(const QVariant &value)
{
    return value.metaType().id() == QMetaType::Bool;
}

inline bool isInteger(const QVariant &value)
{
    return value.metaType().id() == QMetaType::Int;
}

inline bool isReal(const QVariant &value)
{
    return value.metaType().id() == QMetaType::Double;
}

inline bool isString(const QVariant &value)
{
    return value.metaType().id() == QMetaType::QString;
}

inline bool isByteArray(const QVariant &value)
{
    return value.metaType().id() == QMetaType::QByteArray;
}

inline bool isList(const QVariant &value)
{
    return value.metaType().id() == QMetaType::QVariantList;
}

inline bool isMap(const QVariant &value)
{
    return value.metaType().id() == QMetaType::QVariantMap;
}

inline QString string(const QVariant &value, const QString &fallback = QString())
{
    return isString(value) ? value.toString() : fallback;
}

inline QString normalized(const QVariant &value)
{
    return string(value).trimmed().toLower().remove(QRegularExpression(QStringLiteral("[ _-]")));
}

inline QStringList strings(const QVariant &value)
{
    if (!isList(value)) return {};
    QStringList result;
    for (const QVariant &item : value.toList()) {
        if (isString(item)) result.push_back(item.toString());
    }
    return result;
}

inline void dimension(const QVariant &value, int &absolute, int &relative)
{
    if (!isString(value)) return;
    const QString text = string(value).trimmed();
    if (text.endsWith(QLatin1Char('%'))) {
        bool ok = false;
        const int parsed = text.left(text.size() - 1).toInt(&ok);
        if (ok) { relative = parsed; absolute = 0; }
    } else {
        bool ok = false;
        const int parsed = text.toInt(&ok);
        if (ok) { absolute = parsed; relative = 0; }
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping::helpers
