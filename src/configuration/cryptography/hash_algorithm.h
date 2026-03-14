#pragma once

#include "../contracts/cryptography/i_hash_algorithm.h"

namespace seb::configuration::cryptography {

class HashAlgorithm : public contracts::cryptography::IHashAlgorithm
{
public:
    QString generateHashFor(const QString &password) const override;
};

}  // namespace seb::configuration::cryptography
