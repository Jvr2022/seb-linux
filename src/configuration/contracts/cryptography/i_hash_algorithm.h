#pragma once

#include <QString>

namespace seb::configuration::contracts::cryptography {

class IHashAlgorithm
{
public:
    virtual ~IHashAlgorithm() = default;
    virtual QString generateHashFor(const QString &password) const = 0;
};

}  // namespace seb::configuration::contracts::cryptography
