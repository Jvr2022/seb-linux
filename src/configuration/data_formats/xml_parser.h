#pragma once

#include "../contracts/data_formats/i_data_parser.h"

namespace seb::configuration::data_formats {

class XmlParser : public contracts::data_formats::IDataParser
{
public:
    bool canParse(const QByteArray &data) const override;
    contracts::data_formats::ParseResult tryParse(
        const QByteArray &data,
        const contracts::cryptography::PasswordParameters *password = nullptr) const override;
};

}  // namespace seb::configuration::data_formats
