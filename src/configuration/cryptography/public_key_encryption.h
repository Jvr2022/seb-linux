#pragma once

#include "../contracts/cryptography/i_certificate_store.h"
#include "../contracts/cryptography/i_public_key_encryption.h"

namespace seb::configuration::cryptography {

class PublicKeyEncryption : public contracts::cryptography::IPublicKeyEncryption
{
public:
    explicit PublicKeyEncryption(contracts::cryptography::ICertificateStore &store);

    contracts::LoadStatus decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const override;
    contracts::SaveStatus encrypt(const QByteArray &data, const QSslCertificate &certificate, QByteArray &encrypted) const override;

protected:
    contracts::cryptography::ICertificateStore &store_;
};

}  // namespace seb::configuration::cryptography
