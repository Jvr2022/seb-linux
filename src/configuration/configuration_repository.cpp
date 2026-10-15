#include "configuration_repository.h"

#include "configuration_data/data_mapper.h"
#include "configuration_data/data_processor.h"
#include "configuration_data/data_validator.h"
#include "configuration_data/data_values.h"
#include "contracts/cryptography/i_certificate_store.h"
#include "contracts/data_formats/i_data_parser.h"
#include "contracts/data_formats/i_data_serializer.h"
#include "contracts/data_resources/i_resource_loader.h"
#include "contracts/data_resources/i_resource_saver.h"
#include "cryptography/certificate_store.h"
#include "cryptography/hash_algorithm.h"
#include "cryptography/password_encryption.h"
#include "cryptography/public_key_encryption.h"
#include "cryptography/public_key_symmetric_encryption.h"
#include "data_compression/gzip_compressor.h"
#include "data_formats/binary_parser.h"
#include "data_formats/binary_serializer.h"
#include "data_formats/xml_parser.h"
#include "data_formats/xml_serializer.h"
#include "data_formats/internal_keys.h"
#include "data_resources/file_resource_loader.h"
#include "data_resources/file_resource_saver.h"
#include "data_resources/network_resource_loader.h"

namespace seb::configuration {
namespace {

configuration_data::DataMapper &dataMapper()
{
    static configuration_data::DataMapper mapper;
    return mapper;
}

configuration_data::DataProcessor &dataProcessor()
{
    static configuration_data::DataProcessor processor;
    return processor;
}

configuration_data::DataValidator &dataValidator()
{
    static configuration_data::DataValidator validator;
    return validator;
}

configuration_data::DataValues &dataValues()
{
    static configuration_data::DataValues values;
    return values;
}

}  // namespace

ConfigurationRepository::ConfigurationRepository()
{
    certificateStore_ = std::make_unique<cryptography::CertificateStore>();
    compressor_ = std::make_unique<data_compression::GZipCompressor>();
    hashAlgorithm_ = std::make_unique<cryptography::HashAlgorithm>();
    passwordEncryption_ = std::make_unique<cryptography::PasswordEncryption>();
    publicKeyEncryption_ = std::make_unique<cryptography::PublicKeyEncryption>(*certificateStore_);
    publicKeySymmetricEncryption_ = std::make_unique<cryptography::PublicKeySymmetricEncryption>(
        *certificateStore_, *publicKeyEncryption_, *passwordEncryption_);
    xmlParser_ = std::make_unique<data_formats::XmlParser>(*compressor_);
    binaryParser_ = std::make_unique<data_formats::BinaryParser>(
        *compressor_, *hashAlgorithm_, *passwordEncryption_, *publicKeyEncryption_,
        *publicKeySymmetricEncryption_, *xmlParser_);
    registerParser(binaryParser_.get());
    registerParser(xmlParser_.get());
    registerSerializer(new data_formats::BinarySerializer());
    registerSerializer(new data_formats::XmlSerializer());
    registerLoader(new data_resources::FileResourceLoader());
    registerLoader(new data_resources::NetworkResourceLoader(dataValues().initializeAppConfig()));
    registerSaver(new data_resources::FileResourceSaver());
}

ConfigurationRepository::~ConfigurationRepository() = default;

contracts::SaveStatus ConfigurationRepository::configureClientWith(
    const QUrl &resource,
    const contracts::cryptography::PasswordParameters *password)
{
    QByteArray data;
    if (tryLoadData(resource, data) != contracts::LoadStatus::Success) {
        return contracts::SaveStatus::UnexpectedError;
    }

    std::shared_ptr<contracts::cryptography::EncryptionParameters> encryption;
    contracts::data_formats::FormatType format = contracts::data_formats::FormatType::Binary;
    QVariantMap rawData;
    if (tryParseData(data, encryption, format, rawData, password) != contracts::LoadStatus::Success) {
        return contracts::SaveStatus::UnexpectedError;
    }

    certificateStore_->extractAndImportIdentities(rawData);

    QByteArray serialized;
    const contracts::SaveStatus serializeStatus = trySerializeData(rawData, format, serialized, encryption.get());
    if (serializeStatus != contracts::SaveStatus::Success) {
        return serializeStatus;
    }

    return trySaveData(QUrl::fromLocalFile(dataValues().getAppDataFilePath()), serialized);
}

contracts::AppConfig ConfigurationRepository::initializeAppConfig()
{
    return dataValues().initializeAppConfig();
}

contracts::SessionConfiguration ConfigurationRepository::initializeSessionConfiguration()
{
    return dataValues().initializeSessionConfiguration();
}

seb::settings::AppSettings ConfigurationRepository::loadDefaultSettings()
{
    return dataValues().loadDefaultSettings();
}

contracts::LoadStatus ConfigurationRepository::tryLoadSettings(
    const QUrl &resource,
    seb::settings::AppSettings &settings,
    const contracts::cryptography::PasswordParameters *password)
{
    settings = loadDefaultSettings();
    dataProcessor().processDefault(settings);

    QByteArray data;
    const contracts::LoadStatus loadStatus = tryLoadData(resource, data);
    if (loadStatus != contracts::LoadStatus::Success) {
        return loadStatus;
    }

    std::shared_ptr<contracts::cryptography::EncryptionParameters> encryption;
    contracts::data_formats::FormatType format = contracts::data_formats::FormatType::Binary;
    QVariantMap rawData;
    const contracts::LoadStatus parseStatus = tryParseData(data, encryption, format, rawData, password);
    if (parseStatus != contracts::LoadStatus::Success) {
        return parseStatus;
    }

    QVariantMap settingsData = rawData;
    settingsData.remove(data_formats::internal_keys::RawSebData);

    if (!dataValidator().validate(settingsData)) {
        return contracts::LoadStatus::InvalidData;
    }

    dataMapper().map(settingsData, settings);
    dataProcessor().process(settingsData, settings);
    return contracts::LoadStatus::Success;
}

void ConfigurationRepository::registerParser(contracts::data_formats::IDataParser *parser)
{
    dataParsers_.push_back(parser);
}

void ConfigurationRepository::registerSerializer(contracts::data_formats::IDataSerializer *serializer)
{
    dataSerializers_.push_back(serializer);
}

void ConfigurationRepository::registerLoader(contracts::data_resources::IResourceLoader *loader)
{
    resourceLoaders_.push_back(loader);
}

void ConfigurationRepository::registerSaver(contracts::data_resources::IResourceSaver *saver)
{
    resourceSavers_.push_back(saver);
}

contracts::LoadStatus ConfigurationRepository::tryLoadData(const QUrl &resource, QByteArray &data) const
{
    for (const auto *loader : resourceLoaders_) {
        if (!loader->canLoad(resource)) {
            continue;
        }
        const contracts::LoadStatus status = loader->tryLoad(resource, data);
        if (status != contracts::LoadStatus::NotSupported) {
            return status;
        }
    }
    return contracts::LoadStatus::NotSupported;
}

contracts::LoadStatus ConfigurationRepository::tryParseData(
    const QByteArray &data,
    std::shared_ptr<contracts::cryptography::EncryptionParameters> &encryption,
    contracts::data_formats::FormatType &format,
    QVariantMap &rawData,
    const contracts::cryptography::PasswordParameters *password) const
{
    encryption.reset();
    for (const auto *parser : dataParsers_) {
        if (!parser->canParse(data)) {
            continue;
        }
        const auto result = parser->tryParse(data, password);
        encryption = result.encryption;
        format = result.format;
        rawData = result.rawData;
        return result.status;
    }
    return contracts::LoadStatus::NotSupported;
}

contracts::SaveStatus ConfigurationRepository::trySerializeData(
    const QVariantMap &data,
    contracts::data_formats::FormatType format,
    QByteArray &serialized,
    const contracts::cryptography::EncryptionParameters *encryption) const
{
    for (const auto *serializer : dataSerializers_) {
        if (!serializer->canSerialize(format)) {
            continue;
        }
        const auto result = serializer->trySerialize(data, encryption);
        serialized = result.data;
        return result.status;
    }
    return contracts::SaveStatus::NotSupported;
}

contracts::SaveStatus ConfigurationRepository::trySaveData(const QUrl &destination, const QByteArray &data) const
{
    for (const auto *saver : resourceSavers_) {
        if (!saver->canSave(destination)) {
            continue;
        }
        const contracts::SaveStatus status = saver->trySave(destination, data);
        if (status != contracts::SaveStatus::NotSupported) {
            return status;
        }
    }
    return contracts::SaveStatus::NotSupported;
}

}  // namespace seb::configuration
