#include "key_generator.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QFile>
#include <QMessageAuthenticationCode>
#include <QUrl>

#include <cstdlib>

namespace seb::browser {

KeyGenerator::KeyGenerator(const seb::SebSettings &settings)
    : settings_(settings)
{
}

QByteArray KeyGenerator::configurationKeyHash(const QUrl &url) const
{
    return QCryptographicHash::hash(
               (normalizeUrl(url) + settings_.browser.configurationKey).toUtf8(),
               QCryptographicHash::Sha256)
        .toHex();
}

QByteArray KeyGenerator::requestHash(const QUrl &url) const
{
    return QCryptographicHash::hash(
               (normalizeUrl(url) + browserExamKey()).toUtf8(),
               QCryptographicHash::Sha256)
        .toHex();
}

QString KeyGenerator::browserExamKey() const
{
    if (!settings_.browser.customBrowserExamKey.isEmpty()) {
        return settings_.browser.customBrowserExamKey;
    }

    if (browserExamKey_.isEmpty()) {
        const QByteArray salt = settings_.browser.browserExamKeySalt;
        const QString payload = codeSignatureHash() + buildVersion() + settings_.browser.configurationKey;

        browserExamKey_ = QString::fromLatin1(
            QMessageAuthenticationCode::hash(payload.toUtf8(), salt, QCryptographicHash::Sha256).toHex());
    }

    return browserExamKey_;
}

QString KeyGenerator::executableHash() const
{
    if (!executableHash_.isEmpty()) {
        return executableHash_;
    }

    QFile file(QCoreApplication::applicationFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        executableHash_ = QStringLiteral("seb-linux");
        return executableHash_;
    }

    executableHash_ =
        QString::fromLatin1(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Sha256).toHex());
    return executableHash_;
}

QString KeyGenerator::buildVersion() const
{
    const QByteArray override = qgetenv("SEB_PROGRAM_BUILD_VERSION");
    return override.isEmpty()
        ? QCoreApplication::applicationVersion()
        : QString::fromLocal8Bit(override);
}

QString KeyGenerator::codeSignatureHash() const
{
    const QByteArray override = qgetenv("SEB_CODE_SIGNATURE_HASH");
    if (!override.isEmpty()) {
        return QString::fromLatin1(override).trimmed();
    }

    return executableHash();
}

QString KeyGenerator::normalizeUrl(const QUrl &url) const
{
    return url.adjusted(QUrl::RemoveFragment).toString();
}

}  // namespace seb::browser
