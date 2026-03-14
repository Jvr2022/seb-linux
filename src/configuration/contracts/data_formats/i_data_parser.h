#pragma once

#include "../cryptography/password_parameters.h"
#include "parse_result.h"

#include <QByteArray>

namespace seb::configuration::contracts::data_formats {

class IDataParser
{
public:
    virtual ~IDataParser() = default;
    virtual bool canParse(const QByteArray &data) const = 0;
    virtual ParseResult tryParse(const QByteArray &data, const cryptography::PasswordParameters *password = nullptr) const = 0;
};

}  // namespace seb::configuration::contracts::data_formats
