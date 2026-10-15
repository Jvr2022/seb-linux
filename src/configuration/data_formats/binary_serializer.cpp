#include "binary_serializer.h"

#include "binary_block.h"
#include "../contracts/cryptography/public_key_parameters.h"
#include "../contracts/data_compression/i_data_compressor.h"
#include "../contracts/cryptography/i_password_encryption.h"

namespace seb::configuration::data_formats {

BinarySerializer::BinarySerializer(
    const contracts::data_compression::IDataCompressor &compressor,
    const contracts::cryptography::IPasswordEncryption &passwordEncryption)
    : compressor_(compressor)
    , passwordEncryption_(passwordEncryption)
{
}

bool BinarySerializer::canSerialize(contracts::data_formats::FormatType format) const
{
    return format == contracts::data_formats::FormatType::Binary;
}

contracts::data_formats::SerializeResult BinarySerializer::trySerialize(
    const QVariantMap &data,
    const contracts::cryptography::EncryptionParameters *encryption) const
{
    if (const auto *password = dynamic_cast<const contracts::cryptography::PasswordParameters *>(encryption)) {
        return serializePassword(data, *password);
    }

    if (dynamic_cast<const contracts::cryptography::PublicKeyParameters *>(encryption)) {
        contracts::data_formats::SerializeResult result;
        result.status = contracts::SaveStatus::NotSupported;
        return result;
    }

    return serializePlain(data, true);
}

contracts::data_formats::SerializeResult BinarySerializer::serializePlain(
    const QVariantMap &data,
    bool withPrefix,
    bool outerCompression) const
{
    contracts::data_formats::SerializeResult result = xmlSerializer_.trySerialize(data);
    if (result.status != contracts::SaveStatus::Success) {
        return result;
    }

    result.data = compressor_.compress(result.data);
    if (result.data.isEmpty()) {
        result.status = contracts::SaveStatus::UnexpectedError;
        return result;
    }

    if (withPrefix) {
        result.data.prepend(BinaryBlock::PlainData);
    }

    if (outerCompression) {
        result.data = compressor_.compress(result.data);
        if (result.data.isEmpty()) {
            result.status = contracts::SaveStatus::UnexpectedError;
        }
    }
    return result;
}

contracts::data_formats::SerializeResult BinarySerializer::serializePassword(
    const QVariantMap &data,
    const contracts::cryptography::PasswordParameters &password) const
{
    contracts::data_formats::SerializeResult result = serializePlain(data, false, false);
    if (result.status != contracts::SaveStatus::Success) {
        return result;
    }

    QByteArray encrypted;
    result.status = passwordEncryption_.encrypt(result.data, password.password, encrypted);
    if (result.status != contracts::SaveStatus::Success) {
        result.data.clear();
        return result;
    }

    result.data = password.isHash ? BinaryBlock::PasswordConfigureClient : BinaryBlock::Password;
    result.data.append(encrypted);
    result.data = compressor_.compress(result.data);
    if (result.data.isEmpty()) {
        result.status = contracts::SaveStatus::UnexpectedError;
    }
    return result;
}

}  // namespace seb::configuration::data_formats
