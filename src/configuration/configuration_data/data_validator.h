#pragma once

#include <QVariantMap>

namespace seb::configuration::configuration_data {

class DataValidator
{
public:
    bool validate(const QVariantMap &rawData) const;

private:
    bool validatePasswords(const QVariantMap &data) const;
    bool validatePassword(const QString &key, const QVariantMap &data) const;
    bool validateStringValues(const QVariant &value, const QString &key = QString()) const;
    bool validateString(const QString &key, const QString *value = nullptr) const;
};

}  // namespace seb::configuration::configuration_data
