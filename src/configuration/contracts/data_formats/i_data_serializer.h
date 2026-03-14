#pragma once

#include "../cryptography/encryption_parameters.h"
#include "format_type.h"
#include "serialize_result.h"

#include <QVariantMap>

namespace seb::configuration::contracts::data_formats {

class IDataSerializer
{
public:
    virtual ~IDataSerializer() = default;
    virtual bool canSerialize(FormatType format) const = 0;
    virtual SerializeResult trySerialize(const QVariantMap &data, const cryptography::EncryptionParameters *encryption = nullptr) const = 0;
};

}  // namespace seb::configuration::contracts::data_formats
