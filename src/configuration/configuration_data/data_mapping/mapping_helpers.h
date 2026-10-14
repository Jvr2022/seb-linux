#pragma once

#include <QMetaType>
#include <QRegularExpression>
#include <QStringList>
#include <QVariant>

namespace seb::configuration::configuration_data::data_mapping::helpers {

inline bool boolean(const QVariant &value, bool fallback = false)
{
    if (!value.isValid() || value.isNull()) return fallback;
    if (value.metaType().id() == QMetaType::Bool) return value.toBool();
    const QString text = value.toString().trimmed().toLower();
    if (text == QStringLiteral("true") || text == QStringLiteral("yes") || text == QStringLiteral("1")) return true;
    if (text == QStringLiteral("false") || text == QStringLiteral("no") || text == QStringLiteral("0")) return false;
    bool ok = false;
    const int integer = value.toInt(&ok);
    return ok ? integer != 0 : fallback;
}

inline int integer(const QVariant &value, int fallback = 0)
{
    bool ok = false;
    const int result = value.toInt(&ok);
    if (ok) return result;
    const int parsed = value.toString().trimmed().toInt(&ok);
    return ok ? parsed : fallback;
}

inline double real(const QVariant &value, double fallback = 0.0)
{
    bool ok = false;
    const double result = value.toDouble(&ok);
    return ok ? result : fallback;
}

inline QString string(const QVariant &value)
{
    return value.isValid() && !value.isNull() ? value.toString() : QString();
}

inline QString normalized(const QVariant &value)
{
    return string(value).trimmed().toLower().remove(QRegularExpression(QStringLiteral("[ _-]")));
}

inline QStringList strings(const QVariant &value)
{
    if (value.metaType().id() == QMetaType::QStringList) return value.toStringList();
    QStringList result;
    for (const QVariant &item : value.toList()) {
        const QString text = item.toString();
        if (!text.isEmpty()) result.push_back(text);
    }
    return result;
}

inline void dimension(const QVariant &value, int &absolute, int &relative)
{
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
