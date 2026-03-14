#pragma once

#include <QByteArray>
#include <QString>

namespace seb::configuration::contracts::cryptography {

class IKeyGenerator
{
public:
    virtual ~IKeyGenerator() = default;
    virtual QString calculateAppSignatureKey(const QString &connectionToken, const QString &salt) = 0;
    virtual QString calculateBrowserExamKeyHash(const QString &configurationKey, const QByteArray &salt, const QString &url) = 0;
    virtual QString calculateConfigurationKeyHash(const QString &configurationKey, const QString &url) = 0;
    virtual void useCustomBrowserExamKey(const QString &browserExamKey) = 0;
};

}  // namespace seb::configuration::contracts::cryptography
