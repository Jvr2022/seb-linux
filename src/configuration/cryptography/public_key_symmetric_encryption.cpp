#include "public_key_symmetric_encryption.h"

#include <QDataStream>

namespace seb::configuration::cryptography {

PublicKeySymmetricEncryption::PublicKeySymmetricEncryption(
    contracts::cryptography::ICertificateStore &store,
    contracts::cryptography::IPublicKeyEncryption &rsa,
    contracts::cryptography::IPasswordEncryption &aes)
    : store_(store)
    , rsa_(rsa)
    , aes_(aes)
{
}

contracts::LoadStatus PublicKeySymmetricEncryption::decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const
{
    if (data.size() < 24) return contracts::LoadStatus::InvalidData;

    const QByteArray keyHash = data.left(20);

    QDataStream ds(data.mid(20, 4));
    ds.setByteOrder(QDataStream::LittleEndian);
    qint32 encryptedKeyLength = 0;
    ds >> encryptedKeyLength;

    const int remainingDataSize = data.size() - 24;
    if (encryptedKeyLength < 0 || encryptedKeyLength > remainingDataSize) {
        return contracts::LoadStatus::InvalidData;
    }

    const QByteArray encryptedKey = data.mid(24, encryptedKeyLength);
    const QByteArray payload = data.mid(24 + encryptedKeyLength);

    QByteArray rsaBlock = keyHash + encryptedKey;
    QByteArray decryptedKey;
    auto status = rsa_.decrypt(rsaBlock, decryptedKey, certificate);
    if (status != contracts::LoadStatus::Success) return status;

    const QString aesPassword = QString::fromLatin1(decryptedKey.toBase64());

    return aes_.decrypt(payload, aesPassword, decrypted);
}

contracts::SaveStatus PublicKeySymmetricEncryption::encrypt(const QByteArray &, const QSslCertificate &, QByteArray &) const
{
    return contracts::SaveStatus::NotSupported;
}

}  // namespace seb::configuration::cryptography
