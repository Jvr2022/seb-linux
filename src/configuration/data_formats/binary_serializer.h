#pragma once

#include "../contracts/data_formats/i_data_serializer.h"
#include "../contracts/cryptography/password_parameters.h"

namespace seb::configuration::contracts::cryptography {
class IPasswordEncryption;
}
namespace seb::configuration::contracts::data_compression {
class IDataCompressor;
}

#include "xml_serializer.h"

namespace seb::configuration::data_formats {

class BinarySerializer : public contracts::data_formats::IDataSerializer
{
public:
    BinarySerializer(
        const contracts::data_compression::IDataCompressor &compressor,
        const contracts::cryptography::IPasswordEncryption &passwordEncryption);

    bool canSerialize(contracts::data_formats::FormatType format) const override;
    contracts::data_formats::SerializeResult trySerialize(
        const QVariantMap &data,
        const contracts::cryptography::EncryptionParameters *encryption = nullptr) const override;

private:
    contracts::data_formats::SerializeResult serializePlain(
        const QVariantMap &data,
        bool withPrefix,
        bool outerCompression = true) const;
    contracts::data_formats::SerializeResult serializePassword(
        const QVariantMap &data,
        const contracts::cryptography::PasswordParameters &password) const;

    const contracts::data_compression::IDataCompressor &compressor_;
    const contracts::cryptography::IPasswordEncryption &passwordEncryption_;
    XmlSerializer xmlSerializer_;
};

}  // namespace seb::configuration::data_formats
