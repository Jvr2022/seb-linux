#pragma once

#include "../contracts/cryptography/i_certificate_store.h"

#include <QHash>
#include <QString>

namespace seb::configuration::cryptography {

class CertificateStore : public contracts::cryptography::ICertificateStore
{
public:
    CertificateStore();

    bool tryGetCertificateWith(const QByteArray &keyHash, QSslCertificate &certificate) const override;
    bool tryGetPrivateKeyFor(const QSslCertificate &certificate, QSslKey &key) const override;
    void extractAndImportIdentities(QVariantMap &data) override;

private:
    void loadPersistedIdentities();
    bool persistIdentity(const QByteArray &keyHash, const QByteArray &data) const;
    QString identityStoragePath() const;

    QHash<QByteArray, QSslCertificate> certificates_;
    QHash<QByteArray, QSslKey> privateKeys_;
};

}  // namespace seb::configuration::cryptography
