#pragma once

#include "../contracts/cryptography/i_public_key_encryption.h"
#include "../contracts/cryptography/i_password_encryption.h"
#include "../contracts/cryptography/i_certificate_store.h"

namespace seb::configuration::cryptography {

class PublicKeySymmetricEncryption : public contracts::cryptography::IPublicKeyEncryption
{
public:
    PublicKeySymmetricEncryption(
        contracts::cryptography::ICertificateStore &store,
        contracts::cryptography::IPublicKeyEncryption &rsa,
        contracts::cryptography::IPasswordEncryption &aes);

    contracts::LoadStatus decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const override;
    contracts::SaveStatus encrypt(const QByteArray &data, const QSslCertificate &certificate, QByteArray &encrypted) const override;

private:
    contracts::cryptography::ICertificateStore &store_;
    contracts::cryptography::IPublicKeyEncryption &rsa_;
    contracts::cryptography::IPasswordEncryption &aes_;
};

}  // namespace seb::configuration::cryptography
