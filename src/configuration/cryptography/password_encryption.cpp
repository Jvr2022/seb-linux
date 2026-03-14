#include "password_encryption.h"

namespace seb::configuration::cryptography {

contracts::LoadStatus PasswordEncryption::decrypt(const QByteArray &data, const QString &password, QByteArray &decrypted) const
{
    if (password.isNull()) {
        return contracts::LoadStatus::PasswordNeeded;
    }

    decrypted = data;
    return contracts::LoadStatus::Success;
}

contracts::SaveStatus PasswordEncryption::encrypt(const QByteArray &data, const QString &, QByteArray &encrypted) const
{
    encrypted = data;
    return contracts::SaveStatus::Success;
}

}  // namespace seb::configuration::cryptography
