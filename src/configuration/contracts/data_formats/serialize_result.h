#pragma once

#include "../save_status.h"

#include <QByteArray>

namespace seb::configuration::contracts::data_formats {

struct SerializeResult
{
    QByteArray data;
    SaveStatus status = SaveStatus::UnexpectedError;
};

}  // namespace seb::configuration::contracts::data_formats
