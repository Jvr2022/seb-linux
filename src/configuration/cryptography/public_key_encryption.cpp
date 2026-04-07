#include "public_key_encryption.h"

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

namespace seb::configuration::cryptography {

PublicKeyEncryption::PublicKeyEncryption(contracts::cryptography::ICertificateStore &store)
    : store_(store)
{
}

contracts::LoadStatus PublicKeyEncryption::decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const
{
    if (data.size() < 20) return contracts::LoadStatus::InvalidData;

    const QByteArray keyHash = data.left(20);
    if (!store_.tryGetCertificateWith(keyHash, certificate)) {
        return contracts::LoadStatus::InvalidData;
    }

    QSslKey privateKey;
    if (!store_.tryGetPrivateKeyFor(certificate, privateKey)) {
        return contracts::LoadStatus::PasswordNeeded; // Or a more specific status
    }

    const QByteArray cipherData = data.mid(20);
    
    // Convert QSslKey to EVP_PKEY via PEM to avoid fragile handle() casts
    const QByteArray pemKey = privateKey.toPem();
    BIO *bio = BIO_new_mem_buf(pemKey.constData(), pemKey.size());
    if (!bio) return contracts::LoadStatus::UnexpectedError;

    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!pkey) return contracts::LoadStatus::UnexpectedError;

    auto cleanup = [pkey]() {
        EVP_PKEY_free(pkey);
    };

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(pkey, nullptr);
    if (!ctx) {
        cleanup();
        return contracts::LoadStatus::UnexpectedError;
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        cleanup();
        return contracts::LoadStatus::UnexpectedError;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        cleanup();
        return contracts::LoadStatus::UnexpectedError;
    }

    size_t outLen = 0;
    if (EVP_PKEY_decrypt(ctx, nullptr, &outLen, 
                          reinterpret_cast<const unsigned char*>(cipherData.constData()), 
                          cipherData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        cleanup();
        return contracts::LoadStatus::InvalidData;
    }

    decrypted.resize(static_cast<int>(outLen));
    if (EVP_PKEY_decrypt(ctx, reinterpret_cast<unsigned char*>(decrypted.data()), &outLen, 
                          reinterpret_cast<const unsigned char*>(cipherData.constData()), 
                          cipherData.size()) <= 0) {
        EVP_PKEY_CTX_free(ctx);
        cleanup();
        return contracts::LoadStatus::InvalidData;
    }

    decrypted.resize(static_cast<int>(outLen));
    EVP_PKEY_CTX_free(ctx);
    cleanup();

    return contracts::LoadStatus::Success;
}

contracts::SaveStatus PublicKeyEncryption::encrypt(const QByteArray &, const QSslCertificate &, QByteArray &) const
{
    return contracts::SaveStatus::NotSupported;
}

}  // namespace seb::configuration::cryptography
