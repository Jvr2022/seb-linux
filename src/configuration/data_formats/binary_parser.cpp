#include "binary_parser.h"

#include "binary_block.h"
#include "internal_keys.h"
#include "../contracts/cryptography/password_parameters.h"
#include "../contracts/cryptography/public_key_parameters.h"

namespace seb::configuration::data_formats {
namespace {

QByteArray readPrefix(const QByteArray &data, const contracts::data_compression::IDataCompressor &compressor)
{
    return compressor.isCompressed(data) ? compressor.peek(data, 4) : data.left(4);
}

bool isKnownPrefix(const QByteArray &prefix)
{
    return prefix == BinaryBlock::Password || prefix == BinaryBlock::PasswordConfigureClient ||
           prefix == BinaryBlock::PlainData || prefix == BinaryBlock::PublicKey ||
           prefix == BinaryBlock::PublicKeySymmetric;
}

contracts::data_formats::ParseResult parsePlainPayload(
    const QByteArray &payload,
    const contracts::data_compression::IDataCompressor &compressor,
    const contracts::data_formats::IDataParser &xmlParser)
{
    const QByteArray plain = compressor.isCompressed(payload) ? compressor.decompress(payload) : payload;
    if (plain.isEmpty()) {
        contracts::data_formats::ParseResult result;
        result.format = contracts::data_formats::FormatType::Binary;
        result.status = contracts::LoadStatus::InvalidData;
        return result;
    }
    auto result = xmlParser.tryParse(plain);
    result.format = contracts::data_formats::FormatType::Binary;
    return result;
}

}  // namespace

BinaryParser::BinaryParser(
    const contracts::data_compression::IDataCompressor &compressor,
    const contracts::cryptography::IHashAlgorithm &hashAlgorithm,
    const contracts::cryptography::IPasswordEncryption &passwordEncryption,
    const contracts::cryptography::IPublicKeyEncryption &publicKeyEncryption,
    const contracts::cryptography::IPublicKeyEncryption &publicKeySymmetricEncryption,
    const contracts::data_formats::IDataParser &xmlParser)
    : compressor_(compressor),
      hashAlgorithm_(hashAlgorithm),
      passwordEncryption_(passwordEncryption),
      publicKeyEncryption_(publicKeyEncryption),
      publicKeySymmetricEncryption_(publicKeySymmetricEncryption),
      xmlParser_(xmlParser)
{
}

bool BinaryParser::canParse(const QByteArray &data) const
{
    return isKnownPrefix(readPrefix(data, compressor_));
}

contracts::data_formats::ParseResult BinaryParser::tryParse(
    const QByteArray &source,
    const contracts::cryptography::PasswordParameters *password) const
{
    contracts::data_formats::ParseResult invalid;
    invalid.format = contracts::data_formats::FormatType::Binary;
    invalid.status = contracts::LoadStatus::InvalidData;

    QByteArray data = source;
    const QByteArray prefix = readPrefix(source, compressor_);
    if (!isKnownPrefix(prefix)) {
        return invalid;
    }
    if (compressor_.isCompressed(data)) {
        data = compressor_.decompress(data);
        if (data.isEmpty()) {
            return invalid;
        }
    }
    if (!data.startsWith(prefix)) {
        return invalid;
    }
    data.remove(0, 4);

    if (prefix == BinaryBlock::PlainData) {
        auto result = parsePlainPayload(data, compressor_, xmlParser_);
        if (result.status == contracts::LoadStatus::Success) {
            result.rawData.insert(internal_keys::RawSebData, source);
        }
        return result;
    }

    if (prefix == BinaryBlock::Password || prefix == BinaryBlock::PasswordConfigureClient) {
        if (password == nullptr) {
            invalid.status = contracts::LoadStatus::PasswordNeeded;
            return invalid;
        }
        contracts::cryptography::PasswordParameters parameters = *password;
        if (prefix == BinaryBlock::PasswordConfigureClient && !parameters.isHash) {
            parameters.password = hashAlgorithm_.generateHashFor(parameters.password);
            parameters.isHash = true;
        }
        QByteArray decrypted;
        const auto decryptStatus = passwordEncryption_.decrypt(data, parameters.password, decrypted);
        if (decryptStatus != contracts::LoadStatus::Success) {
            invalid.status = decryptStatus;
            return invalid;
        }
        auto result = parsePlainPayload(decrypted, compressor_, xmlParser_);
        if (result.status == contracts::LoadStatus::Success) {
            result.rawData.insert(internal_keys::RawSebData, source);
        }
        result.encryption = std::make_shared<contracts::cryptography::PasswordParameters>(parameters);
        return result;
    }

    const bool symmetric = prefix == BinaryBlock::PublicKeySymmetric;
    const auto &encryption = symmetric ? publicKeySymmetricEncryption_ : publicKeyEncryption_;
    QByteArray decrypted;
    QSslCertificate certificate;
    if (encryption.decrypt(data, decrypted, certificate) != contracts::LoadStatus::Success) {
        return invalid;
    }

    auto result = tryParse(decrypted, password);
    if (result.status == contracts::LoadStatus::Success) {
        result.rawData.insert(internal_keys::RawSebData, source);
    }
    auto parameters = std::make_shared<contracts::cryptography::PublicKeyParameters>();
    parameters->certificate = certificate;
    parameters->symmetricEncryption = symmetric;
    if (const auto inner = std::dynamic_pointer_cast<contracts::cryptography::PasswordParameters>(result.encryption)) {
        parameters->innerEncryption = *inner;
    }
    result.encryption = std::move(parameters);
    return result;
}

}  // namespace seb::configuration::data_formats
