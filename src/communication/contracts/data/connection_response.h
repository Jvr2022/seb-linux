#pragma once

#include "response.h"

#include <QUuid>

namespace seb::communication::contracts::data {

struct ConnectionResponse : Response
{
    QUuid communicationToken;
    bool connectionEstablished = false;
};

}  // namespace seb::communication::contracts::data
