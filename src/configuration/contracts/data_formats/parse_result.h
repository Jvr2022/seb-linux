#pragma once

#include "../cryptography/encryption_parameters.h"
#include "../load_status.h"
#include "format_type.h"

#include <QVariantMap>

#include <memory>

namespace seb::configuration::contracts::data_formats {

struct ParseResult
{
    std::shared_ptr<cryptography::EncryptionParameters> encryption;
    FormatType format = FormatType::Xml;
    QVariantMap rawData;
    LoadStatus status = LoadStatus::UnexpectedError;
};

}  // namespace seb::configuration::contracts::data_formats
