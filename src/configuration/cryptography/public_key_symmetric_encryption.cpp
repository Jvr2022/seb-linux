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
    if (data.size() < 24) return contracts::LoadStatus::InvalidData; // 20 (hash) + 4 (length)

    // First 20 bytes are public key hash
    const QByteArray keyHash = data.left(20);
    
    // Next 4 bytes are encrypted key length (little endian int)
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

    // Decrypt the symmetric key using RSA
    // We need the keyHash block to identify the cert, so we prepend it for the rsa_.decrypt call
    QByteArray rsaBlock = keyHash + encryptedKey;
    QByteArray decryptedKey;
    auto status = rsa_.decrypt(rsaBlock, decryptedKey, certificate);
    if (status != contracts::LoadStatus::Success) return status;

    // The decrypted key is likely a base64 string or raw bytes. 
    // SEB Windows converts it to base64 before using it as the password for PasswordEncryption.
    // Wait, let's look at PasswordSymmetricEncryption.cs line 102:
    // var symmetricKey = Convert.ToBase64String(decryptedKey.ToArray());
    // So we use the base64 version.
    const QString aesPassword = QString::fromLatin1(decryptedKey.toBase64());

    // Decrypt the payload using AES
    return aes_.decrypt(payload, aesPassword, decrypted);
}

contracts::SaveStatus PublicKeySymmetricEncryption::encrypt(const QByteArray &, const QSslCertificate &, QByteArray &) const
{
    return contracts::SaveStatus::NotSupported;
}

}  // namespace seb::configuration::cryptography
