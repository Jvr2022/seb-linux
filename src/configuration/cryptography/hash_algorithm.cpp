#include "hash_algorithm.h"

#include <QCryptographicHash>

namespace seb::configuration::cryptography {

QString HashAlgorithm::generateHashFor(const QString &password) const
{
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

}  // namespace seb::configuration::cryptography
