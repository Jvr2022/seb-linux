#include "sub_stream.h"

namespace seb::configuration {

SubStream::SubStream(const QByteArray &data, int offset, int length)
{
    if (offset < 0) {
        offset = 0;
    }
    data_ = length < 0 ? data.mid(offset) : data.mid(offset, length);
}

QByteArray SubStream::readAll() const
{
    return data_;
}

}  // namespace seb::configuration
