#pragma once

#include <QByteArray>

namespace seb::configuration {

class SubStream
{
public:
    SubStream(const QByteArray &data, int offset = 0, int length = -1);
    QByteArray readAll() const;

private:
    QByteArray data_;
};

}  // namespace seb::configuration
