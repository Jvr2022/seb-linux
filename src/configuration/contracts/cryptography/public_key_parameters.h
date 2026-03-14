#pragma once

#include "encryption_parameters.h"
#include "password_parameters.h"

#include <QSslCertificate>

namespace seb::configuration::contracts::cryptography {

struct PublicKeyParameters : EncryptionParameters
{
    QSslCertificate certificate;
    PasswordParameters innerEncryption;
    bool symmetricEncryption = false;
};

}  // namespace seb::configuration::contracts::cryptography
