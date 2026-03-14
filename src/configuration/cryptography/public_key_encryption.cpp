#include "public_key_encryption.h"

namespace seb::configuration::cryptography {

PublicKeyEncryption::PublicKeyEncryption(contracts::cryptography::ICertificateStore &store)
    : store_(store)
{
}

contracts::LoadStatus PublicKeyEncryption::decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const
{
    if (!store_.tryGetCertificateWith(data.left(20), certificate)) {
        return contracts::LoadStatus::InvalidData;
    }

    decrypted = data.mid(20);
    return contracts::LoadStatus::Success;
}

contracts::SaveStatus PublicKeyEncryption::encrypt(const QByteArray &data, const QSslCertificate &, QByteArray &encrypted) const
{
    encrypted = data;
    return contracts::SaveStatus::Success;
}

}  // namespace seb::configuration::cryptography
