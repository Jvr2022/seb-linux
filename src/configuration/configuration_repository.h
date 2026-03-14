#pragma once

#include "contracts/i_configuration_repository.h"
#include "contracts/data_formats/format_type.h"

#include <QList>
#include <QVariantMap>

namespace seb::configuration {

namespace contracts::cryptography {
class ICertificateStore;
}
namespace contracts::data_formats {
class IDataParser;
class IDataSerializer;
}
namespace contracts::data_resources {
class IResourceLoader;
class IResourceSaver;
}

class ConfigurationRepository : public contracts::IConfigurationRepository
{
public:
    ConfigurationRepository();

    contracts::SaveStatus configureClientWith(const QUrl &resource, const contracts::cryptography::PasswordParameters *password = nullptr) override;
    contracts::AppConfig initializeAppConfig() override;
    contracts::SessionConfiguration initializeSessionConfiguration() override;
    seb::SebSettings loadDefaultSettings() override;
    contracts::LoadStatus tryLoadSettings(const QUrl &resource, seb::SebSettings &settings, const contracts::cryptography::PasswordParameters *password = nullptr) override;

    void registerParser(contracts::data_formats::IDataParser *parser);
    void registerSerializer(contracts::data_formats::IDataSerializer *serializer);
    void registerLoader(contracts::data_resources::IResourceLoader *loader);
    void registerSaver(contracts::data_resources::IResourceSaver *saver);

private:
    contracts::LoadStatus tryLoadData(const QUrl &resource, QByteArray &data) const;
    contracts::LoadStatus tryParseData(
        const QByteArray &data,
        contracts::cryptography::EncryptionParameters *&encryption,
        contracts::data_formats::FormatType &format,
        QVariantMap &rawData,
        const contracts::cryptography::PasswordParameters *password = nullptr) const;
    contracts::SaveStatus trySerializeData(
        const QVariantMap &data,
        contracts::data_formats::FormatType format,
        QByteArray &serialized,
        const contracts::cryptography::EncryptionParameters *encryption = nullptr) const;
    contracts::SaveStatus trySaveData(const QUrl &destination, const QByteArray &data) const;

    QList<contracts::data_formats::IDataParser *> dataParsers_;
    QList<contracts::data_formats::IDataSerializer *> dataSerializers_;
    QList<contracts::data_resources::IResourceLoader *> resourceLoaders_;
    QList<contracts::data_resources::IResourceSaver *> resourceSavers_;
    contracts::cryptography::ICertificateStore *certificateStore_ = nullptr;
};

}  // namespace seb::configuration
