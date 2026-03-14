#pragma once

#include "../contracts/data_formats/i_data_serializer.h"

namespace seb::configuration::data_formats {

class BinarySerializer : public contracts::data_formats::IDataSerializer
{
public:
    bool canSerialize(contracts::data_formats::FormatType format) const override;
    contracts::data_formats::SerializeResult trySerialize(
        const QVariantMap &data,
        const contracts::cryptography::EncryptionParameters *encryption = nullptr) const override;
};

}  // namespace seb::configuration::data_formats
