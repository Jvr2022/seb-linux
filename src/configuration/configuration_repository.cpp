#include "configuration_repository.h"

#include "configuration_data/data_mapper.h"
#include "configuration_data/data_processor.h"
#include "configuration_data/data_values.h"
#include "contracts/cryptography/i_certificate_store.h"
#include "contracts/data_formats/i_data_parser.h"
#include "contracts/data_formats/i_data_serializer.h"
#include "contracts/data_resources/i_resource_loader.h"
#include "contracts/data_resources/i_resource_saver.h"
#include "cryptography/certificate_store.h"
#include "data_formats/binary_parser.h"
#include "data_formats/binary_serializer.h"
#include "data_formats/xml_parser.h"
#include "data_formats/xml_serializer.h"
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

configuration_data::DataValues &dataValues()
{
    static configuration_data::DataValues values;
    return values;
}

}  // namespace

ConfigurationRepository::ConfigurationRepository()
{
    certificateStore_ = new cryptography::CertificateStore();
    registerParser(new data_formats::BinaryParser());
    registerParser(new data_formats::XmlParser());
    registerSerializer(new data_formats::BinarySerializer());
    registerSerializer(new data_formats::XmlSerializer());
    registerLoader(new data_resources::FileResourceLoader());
    registerLoader(new data_resources::NetworkResourceLoader(dataValues().initializeAppConfig()));
    registerSaver(new data_resources::FileResourceSaver());
}

contracts::SaveStatus ConfigurationRepository::configureClientWith(
    const QUrl &resource,
    const contracts::cryptography::PasswordParameters *password)
{
    QByteArray data;
    if (tryLoadData(resource, data) != contracts::LoadStatus::Success) {
        return contracts::SaveStatus::UnexpectedError;
    }

    contracts::cryptography::EncryptionParameters *encryption = nullptr;
    contracts::data_formats::FormatType format = contracts::data_formats::FormatType::Binary;
    QVariantMap rawData;
    if (tryParseData(data, encryption, format, rawData, password) != contracts::LoadStatus::Success) {
        return contracts::SaveStatus::UnexpectedError;
    }

    if (certificateStore_ != nullptr) {
        certificateStore_->extractAndImportIdentities(rawData);
    }

    QByteArray serialized;
    const contracts::SaveStatus serializeStatus = trySerializeData(rawData, format, serialized, encryption);
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

seb::SebSettings ConfigurationRepository::loadDefaultSettings()
{
    return dataValues().loadDefaultSettings();
}

contracts::LoadStatus ConfigurationRepository::tryLoadSettings(
    const QUrl &resource,
    seb::SebSettings &settings,
    const contracts::cryptography::PasswordParameters *password)
{
    settings = loadDefaultSettings();
    dataProcessor().processDefault(settings);

    QByteArray data;
    const contracts::LoadStatus loadStatus = tryLoadData(resource, data);
    if (loadStatus != contracts::LoadStatus::Success) {
        return loadStatus;
    }

    contracts::cryptography::EncryptionParameters *encryption = nullptr;
    contracts::data_formats::FormatType format = contracts::data_formats::FormatType::Binary;
    QVariantMap rawData;
    const contracts::LoadStatus parseStatus = tryParseData(data, encryption, format, rawData, password);
    if (parseStatus != contracts::LoadStatus::Success) {
        return parseStatus;
    }

    dataMapper().map(rawData, settings);
    dataProcessor().process(rawData, settings);
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
    contracts::cryptography::EncryptionParameters *&encryption,
    contracts::data_formats::FormatType &format,
    QVariantMap &rawData,
    const contracts::cryptography::PasswordParameters *password) const
{
    encryption = nullptr;
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
