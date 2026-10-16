#include "key_generator.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QMutexLocker>

namespace seb::configuration::cryptography {

namespace {

QString urlWithoutFragment(const QString &url)
{
    const qsizetype fragment = url.indexOf(QLatin1Char('#'));
    return fragment < 0 ? url : url.left(fragment);
}

}  // namespace

KeyGenerator::KeyGenerator(const contracts::AppConfig &appConfig)
    : appConfig_(appConfig)
{
}

std::optional<QString> KeyGenerator::calculateAppSignatureKey(
    const QString &connectionToken,
    const QString &salt)
{
    Q_UNUSED(connectionToken);
    Q_UNUSED(salt);
    return std::nullopt;
}

QString KeyGenerator::calculateBrowserExamKeyHash(const QString &configurationKey, const QByteArray &salt, const QString &url)
{
    QMutexLocker locker(&mutex_);
    const QString key = browserExamKey_.isEmpty()
        ? computeBrowserExamKey(configurationKey, salt)
        : browserExamKey_;
    return hashBytes((urlWithoutFragment(url) + key).toUtf8());
}

QString KeyGenerator::calculateConfigurationKeyHash(const QString &configurationKey, const QString &url)
{
    return hashBytes((urlWithoutFragment(url) + configurationKey).toUtf8());
}

void KeyGenerator::useCustomBrowserExamKey(const QString &browserExamKey)
{
    if (!browserExamKey.isEmpty()) {
        QMutexLocker locker(&mutex_);
        browserExamKey_ = browserExamKey;
    }
}

QString KeyGenerator::computeBrowserExamKey(const QString &configurationKey, const QByteArray &salt)
{
    if (browserExamKey_.isEmpty()) {
        const QByteArray input = (appConfig_.codeSignatureHash + appConfig_.programBuildVersion + configurationKey).toUtf8();
        browserExamKey_ = QString::fromLatin1(QMessageAuthenticationCode::hash(input, salt, QCryptographicHash::Sha256).toHex());
    }
    return browserExamKey_;
}

QString KeyGenerator::hashBytes(const QByteArray &data)
{
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

}  // namespace seb::configuration::cryptography
