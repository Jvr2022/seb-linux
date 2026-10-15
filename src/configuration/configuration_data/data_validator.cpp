#include "data_validator.h"

#include "keys.h"

#include <QDebug>
#include <QMetaType>
#include <QRegularExpression>

namespace seb::configuration::configuration_data {
namespace {

const QRegularExpression &passwordHashPattern()
{
    static const QRegularExpression pattern(
        QStringLiteral(R"(\A(?:[0-9a-fA-F]{64})?\z)"));
    return pattern;
}

const QRegularExpression &invalidStringPattern()
{
    static const QRegularExpression pattern(QStringLiteral(R"("\s*,)"));
    return pattern;
}

}  // namespace

bool DataValidator::validate(const QVariantMap &rawData) const
{
    bool valid = true;
    valid &= validatePasswords(rawData);
    valid &= validateStringValues(rawData);

    if (valid) {
        qDebug() << "SEB configuration data is valid.";
    } else {
        qWarning() << "SEB configuration data is invalid and cannot be applied.";
    }

    return valid;
}

bool DataValidator::validatePasswords(const QVariantMap &data) const
{
    bool valid = true;
    valid &= validatePassword(keys::Security::AdminPasswordHash, data);
    valid &= validatePassword(keys::Security::QuitPasswordHash, data);
    valid &= validatePassword(keys::Server::FallbackPasswordHash, data);
    return valid;
}

bool DataValidator::validatePassword(const QString &key, const QVariantMap &data) const
{
    const auto it = data.constFind(key);
    if (it == data.cend()) {
        return true;
    }

    const QVariant &value = it.value();
    if (!value.isValid() || value.isNull()) {
        return true;
    }

    if (value.metaType().id() != QMetaType::QString) {
        qWarning() << "Invalid SEB password hash type for" << key;
        return false;
    }

    if (!passwordHashPattern().match(value.toString()).hasMatch()) {
        qWarning() << "Invalid SEB password hash for" << key;
        return false;
    }

    return true;
}

bool DataValidator::validateStringValues(const QVariant &value, const QString &key) const
{
    bool valid = true;
    const int type = value.metaType().id();

    if (type == QMetaType::QVariantList) {
        const QVariantList list = value.toList();
        for (const QVariant &item : list) {
            valid &= validateStringValues(item, key);
        }
    }

    if (type == QMetaType::QVariantMap) {
        const QVariantMap map = value.toMap();
        for (auto it = map.cbegin(); it != map.cend(); ++it) {
            valid &= validateString(it.key());
            valid &= validateStringValues(it.value(), it.key());
        }
    }

    if (type == QMetaType::QString) {
        const QString string = value.toString();
        valid &= validateString(key, &string);
    }

    return valid;
}

bool DataValidator::validateString(const QString &key, const QString *value) const
{
    const QString &text = value == nullptr ? key : *value;
    if (!invalidStringPattern().match(text).hasMatch()) {
        return true;
    }

    if (value == nullptr) {
        qWarning() << "Invalid SEB configuration key contains prohibited sequence.";
    } else {
        qWarning() << "Invalid SEB string value for key" << key
                   << "contains prohibited sequence.";
    }
    return false;
}

}  // namespace seb::configuration::configuration_data
