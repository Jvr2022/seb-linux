#pragma once

#include <QByteArray>
#include <QHash>
#include <QString>

namespace seb::server::contracts {

struct ServerResponse
{
    bool success = false;
    int statusCode = 0;
    QString message;
    QByteArray body;
    QHash<QByteArray, QByteArray> headers;
};

}  // namespace seb::server::contracts
