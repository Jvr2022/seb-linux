#pragma once

#include "../load_status.h"
#include "../save_status.h"

#include <QByteArray>
#include <QSslCertificate>

namespace seb::configuration::contracts::cryptography {

class IPublicKeyEncryption
{
public:
    virtual ~IPublicKeyEncryption() = default;
    virtual contracts::LoadStatus decrypt(const QByteArray &data, QByteArray &decrypted, QSslCertificate &certificate) const = 0;
    virtual contracts::SaveStatus encrypt(const QByteArray &data, const QSslCertificate &certificate, QByteArray &encrypted) const = 0;
};

}  // namespace seb::configuration::contracts::cryptography
