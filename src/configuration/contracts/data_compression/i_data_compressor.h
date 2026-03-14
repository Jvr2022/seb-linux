#pragma once

#include <QByteArray>

namespace seb::configuration::contracts::data_compression {

class IDataCompressor
{
public:
    virtual ~IDataCompressor() = default;
    virtual QByteArray compress(const QByteArray &data) const = 0;
    virtual QByteArray decompress(const QByteArray &data) const = 0;
    virtual bool isCompressed(const QByteArray &data) const = 0;
    virtual QByteArray peek(const QByteArray &data, int count) const = 0;
};

}  // namespace seb::configuration::contracts::data_compression
