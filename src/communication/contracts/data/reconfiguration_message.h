#pragma once

#include "message.h"

#include <QString>

namespace seb::communication::contracts::data {

struct ReconfigurationMessage : Message
{
    ReconfigurationMessage() = default;
    ReconfigurationMessage(QString path, QString url)
        : configurationPath(std::move(path)), resourceUrl(std::move(url)) {}

    QString configurationPath;
    QString resourceUrl;
};

}  // namespace seb::communication::contracts::data
