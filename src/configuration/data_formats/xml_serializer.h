#pragma once

#include "../contracts/data_formats/i_data_serializer.h"

class QXmlStreamWriter;

namespace seb::configuration::data_formats {

class XmlSerializer : public contracts::data_formats::IDataSerializer
{
public:
    bool canSerialize(contracts::data_formats::FormatType format) const override;
    contracts::data_formats::SerializeResult trySerialize(
        const QVariantMap &data,
        const contracts::cryptography::EncryptionParameters *encryption = nullptr) const override;

private:
    static void writeValue(QXmlStreamWriter &writer, const QVariant &value);
    static void writeDictionary(QXmlStreamWriter &writer, const QVariantMap &data);
};

}  // namespace seb::configuration::data_formats
