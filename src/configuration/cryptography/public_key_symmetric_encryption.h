#pragma once

#include "../contracts/cryptography/public_key_parameters.h"

#include <QByteArray>

namespace seb::configuration::cryptography {

class PublicKeySymmetricEncryption
{
public:
    QByteArray decrypt(const QByteArray &data, const contracts::cryptography::PublicKeyParameters &parameters, bool *ok = nullptr) const;
    QByteArray encrypt(const QByteArray &data, const contracts::cryptography::PublicKeyParameters &parameters, bool *ok = nullptr) const;
};

}  // namespace seb::configuration::cryptography
