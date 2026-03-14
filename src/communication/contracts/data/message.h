#pragma once

#include <QUuid>

namespace seb::communication::contracts::data {

struct Message
{
    virtual ~Message() = default;
    QUuid communicationToken;
};

}  // namespace seb::communication::contracts::data
