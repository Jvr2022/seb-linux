#pragma once

#include "../contracts/data_formats/i_data_parser.h"
#include "../contracts/data_compression/i_data_compressor.h"

namespace seb::configuration::data_formats {

class XmlParser : public contracts::data_formats::IDataParser
{
public:
    explicit XmlParser(const contracts::data_compression::IDataCompressor &compressor);

    bool canParse(const QByteArray &data) const override;
    contracts::data_formats::ParseResult tryParse(
        const QByteArray &data,
        const contracts::cryptography::PasswordParameters *password = nullptr) const override;

private:
    const contracts::data_compression::IDataCompressor &compressor_;
};

}  // namespace seb::configuration::data_formats
