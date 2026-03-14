#pragma once

#include "../load_status.h"
#include "../save_status.h"

#include <QByteArray>
#include <QString>

namespace seb::configuration::contracts::cryptography {

class IPasswordEncryption
{
public:
    virtual ~IPasswordEncryption() = default;
    virtual contracts::LoadStatus decrypt(const QByteArray &data, const QString &password, QByteArray &decrypted) const = 0;
    virtual contracts::SaveStatus encrypt(const QByteArray &data, const QString &password, QByteArray &encrypted) const = 0;
};

}  // namespace seb::configuration::contracts::cryptography
