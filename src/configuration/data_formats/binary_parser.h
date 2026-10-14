#pragma once

#include "../contracts/data_formats/i_data_parser.h"
#include "../contracts/cryptography/i_hash_algorithm.h"
#include "../contracts/cryptography/i_password_encryption.h"
#include "../contracts/cryptography/i_public_key_encryption.h"
#include "../contracts/data_compression/i_data_compressor.h"

namespace seb::configuration::data_formats {

class BinaryParser : public contracts::data_formats::IDataParser
{
public:
    BinaryParser(
        const contracts::data_compression::IDataCompressor &compressor,
        const contracts::cryptography::IHashAlgorithm &hashAlgorithm,
        const contracts::cryptography::IPasswordEncryption &passwordEncryption,
        const contracts::cryptography::IPublicKeyEncryption &publicKeyEncryption,
        const contracts::cryptography::IPublicKeyEncryption &publicKeySymmetricEncryption,
        const contracts::data_formats::IDataParser &xmlParser);

    bool canParse(const QByteArray &data) const override;
    contracts::data_formats::ParseResult tryParse(
        const QByteArray &data,
        const contracts::cryptography::PasswordParameters *password = nullptr) const override;

private:
    const contracts::data_compression::IDataCompressor &compressor_;
    const contracts::cryptography::IHashAlgorithm &hashAlgorithm_;
    const contracts::cryptography::IPasswordEncryption &passwordEncryption_;
    const contracts::cryptography::IPublicKeyEncryption &publicKeyEncryption_;
    const contracts::cryptography::IPublicKeyEncryption &publicKeySymmetricEncryption_;
    const contracts::data_formats::IDataParser &xmlParser_;
};

}  // namespace seb::configuration::data_formats
