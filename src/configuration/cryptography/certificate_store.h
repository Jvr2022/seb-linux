#pragma once

#include "../contracts/cryptography/i_certificate_store.h"

#include <QHash>
#include <QList>
#include <QPair>

namespace seb::configuration::cryptography {

class CertificateStore : public contracts::cryptography::ICertificateStore
{
public:
    bool tryGetCertificateWith(const QByteArray &keyHash, QSslCertificate &certificate) const override;
    bool tryGetPrivateKeyFor(const QSslCertificate &certificate, QSslKey &key) const override;
    void extractAndImportIdentities(const QVariantMap &data) override;

private:
    QHash<QByteArray, QSslCertificate> certificates_;
    QList<QPair<QSslCertificate, QSslKey>> identities_;
};

}  // namespace seb::configuration::cryptography
