#pragma once

#include "../contracts/app_config.h"
#include "../contracts/cryptography/i_key_generator.h"

namespace seb::configuration::cryptography {

class KeyGenerator : public contracts::cryptography::IKeyGenerator
{
public:
    explicit KeyGenerator(const contracts::AppConfig &appConfig);

    QString calculateAppSignatureKey(const QString &connectionToken, const QString &salt) override;
    QString calculateBrowserExamKeyHash(const QString &configurationKey, const QByteArray &salt, const QString &url) override;
    QString calculateConfigurationKeyHash(const QString &configurationKey, const QString &url) override;
    void useCustomBrowserExamKey(const QString &browserExamKey) override;

private:
    QString computeBrowserExamKey(const QString &configurationKey, const QByteArray &salt);
    static QString hashBytes(const QByteArray &data);

    contracts::AppConfig appConfig_;
    QString browserExamKey_;
};

}  // namespace seb::configuration::cryptography
