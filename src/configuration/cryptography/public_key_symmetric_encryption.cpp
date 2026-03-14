#include "public_key_symmetric_encryption.h"

namespace seb::configuration::cryptography {

QByteArray PublicKeySymmetricEncryption::decrypt(const QByteArray &data, const contracts::cryptography::PublicKeyParameters &, bool *ok) const
{
    if (ok) {
        *ok = true;
    }
    return data;
}

QByteArray PublicKeySymmetricEncryption::encrypt(const QByteArray &data, const contracts::cryptography::PublicKeyParameters &, bool *ok) const
{
    if (ok) {
        *ok = true;
    }
    return data;
}

}  // namespace seb::configuration::cryptography
