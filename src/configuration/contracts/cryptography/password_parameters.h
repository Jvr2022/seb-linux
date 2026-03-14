#pragma once

#include "encryption_parameters.h"

#include <QString>

namespace seb::configuration::contracts::cryptography {

struct PasswordParameters : EncryptionParameters
{
    QString password;
    bool isHash = false;
};

}  // namespace seb::configuration::contracts::cryptography
