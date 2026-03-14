#include "xml_parser.h"

#include "../configuration_data/keys.h"

namespace seb::configuration::data_formats {

bool XmlParser::canParse(const QByteArray &data) const
{
    return data.trimmed().startsWith('<');
}

contracts::data_formats::ParseResult XmlParser::tryParse(const QByteArray &data, const contracts::cryptography::PasswordParameters *) const
{
    contracts::data_formats::ParseResult result;
    result.format = contracts::data_formats::FormatType::Xml;
    result.rawData.insert(configuration_data::keys::Meta::RawSebData, data);
    result.rawData.insert(configuration_data::keys::Meta::SourceText, QString::fromUtf8(data));
    result.status = contracts::LoadStatus::Success;
    return result;
}

}  // namespace seb::configuration::data_formats
