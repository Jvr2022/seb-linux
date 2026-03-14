#pragma once

#include "../contracts/cryptography/i_password_encryption.h"

namespace seb::configuration::cryptography {

class PasswordEncryption : public contracts::cryptography::IPasswordEncryption
{
public:
    contracts::LoadStatus decrypt(const QByteArray &data, const QString &password, QByteArray &decrypted) const override;
    contracts::SaveStatus encrypt(const QByteArray &data, const QString &password, QByteArray &encrypted) const override;
};

}  // namespace seb::configuration::cryptography
