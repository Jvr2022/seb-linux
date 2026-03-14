#pragma once

#include <QByteArray>
#include <QSslCertificate>
#include <QVariantMap>

namespace seb::configuration::contracts::cryptography {

class ICertificateStore
{
public:
    virtual ~ICertificateStore() = default;
    virtual bool tryGetCertificateWith(const QByteArray &keyHash, QSslCertificate &certificate) const = 0;
    virtual void extractAndImportIdentities(const QVariantMap &data) = 0;
};

}  // namespace seb::configuration::contracts::cryptography
