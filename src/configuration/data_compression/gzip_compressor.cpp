#include "gzip_compressor.h"

#include <QByteArray>

namespace seb::configuration::data_compression {

QByteArray GZipCompressor::compress(const QByteArray &data) const
{
    return qCompress(data, 9);
}

QByteArray GZipCompressor::decompress(const QByteArray &data) const
{
    return qUncompress(data);
}

bool GZipCompressor::isCompressed(const QByteArray &data) const
{
    return data.size() > 2 && static_cast<unsigned char>(data[0]) == 0x1f && static_cast<unsigned char>(data[1]) == 0x8b;
}

QByteArray GZipCompressor::peek(const QByteArray &data, int count) const
{
    return decompress(data).left(count);
}

}  // namespace seb::configuration::data_compression
