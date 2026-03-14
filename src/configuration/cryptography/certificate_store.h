#pragma once

#include "../contracts/cryptography/i_certificate_store.h"

#include <QHash>

namespace seb::configuration::cryptography {

class CertificateStore : public contracts::cryptography::ICertificateStore
{
public:
    bool tryGetCertificateWith(const QByteArray &keyHash, QSslCertificate &certificate) const override;
    void extractAndImportIdentities(const QVariantMap &data) override;

private:
    QHash<QByteArray, QSslCertificate> certificates_;
};

}  // namespace seb::configuration::cryptography
