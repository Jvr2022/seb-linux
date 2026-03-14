#include "xml_serializer.h"

#include "../configuration_data/json.h"
#include "../configuration_data/keys.h"

#include <QJsonDocument>

namespace seb::configuration::data_formats {

bool XmlSerializer::canSerialize(contracts::data_formats::FormatType format) const
{
    return format == contracts::data_formats::FormatType::Xml;
}

contracts::data_formats::SerializeResult XmlSerializer::trySerialize(const QVariantMap &data, const contracts::cryptography::EncryptionParameters *) const
{
    contracts::data_formats::SerializeResult result;
    const QByteArray raw = data.value(configuration_data::keys::Meta::RawSebData).toByteArray();
    if (!raw.isEmpty()) {
        result.data = raw;
        result.status = contracts::SaveStatus::Success;
        return result;
    }

    const QByteArray json = QJsonDocument(configuration_data::variantMapToJson(data)).toJson(QJsonDocument::Indented);
    result.data = "<seb>\n<![CDATA[\n" + json + "]]>\n</seb>\n";
    result.status = contracts::SaveStatus::Success;
    return result;
}

}  // namespace seb::configuration::data_formats
