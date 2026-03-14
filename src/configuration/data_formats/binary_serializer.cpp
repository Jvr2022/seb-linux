#include "binary_serializer.h"

#include "../configuration_data/keys.h"

namespace seb::configuration::data_formats {

bool BinarySerializer::canSerialize(contracts::data_formats::FormatType format) const
{
    return format == contracts::data_formats::FormatType::Binary;
}

contracts::data_formats::SerializeResult BinarySerializer::trySerialize(const QVariantMap &data, const contracts::cryptography::EncryptionParameters *) const
{
    contracts::data_formats::SerializeResult result;
    result.data = data.value(configuration_data::keys::Meta::RawSebData).toByteArray();
    result.status = result.data.isEmpty() ? contracts::SaveStatus::InvalidData : contracts::SaveStatus::Success;
    return result;
}

}  // namespace seb::configuration::data_formats
