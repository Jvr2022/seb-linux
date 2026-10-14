#include "gzip_compressor.h"

#include <zlib.h>

#include <algorithm>
#include <array>
#include <limits>

namespace seb::configuration::data_compression {
namespace {

constexpr int kGzipWindowBits = MAX_WBITS + 16;
constexpr int kChunkSize = 16 * 1024;

bool hasGzipHeader(const QByteArray &data)
{
    return data.size() >= 3 &&
           static_cast<unsigned char>(data.at(0)) == 0x1f &&
           static_cast<unsigned char>(data.at(1)) == 0x8b &&
           static_cast<unsigned char>(data.at(2)) == Z_DEFLATED;
}

}  // namespace

QByteArray GZipCompressor::compress(const QByteArray &data) const
{
    if (data.size() > static_cast<qsizetype>(std::numeric_limits<uInt>::max())) {
        return {};
    }

    z_stream stream{};
    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, kGzipWindowBits,
                     MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK) {
        return {};
    }

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.constData()));
    stream.avail_in = static_cast<uInt>(data.size());

    QByteArray output;
    std::array<char, kChunkSize> buffer{};
    int status = Z_OK;
    do {
        stream.next_out = reinterpret_cast<Bytef *>(buffer.data());
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = deflate(&stream, Z_FINISH);
        if (status != Z_OK && status != Z_STREAM_END) {
            deflateEnd(&stream);
            return {};
        }
        output.append(buffer.data(), static_cast<qsizetype>(buffer.size() - stream.avail_out));
    } while (status != Z_STREAM_END);

    deflateEnd(&stream);
    return output;
}

QByteArray GZipCompressor::decompress(const QByteArray &data) const
{
    if (!hasGzipHeader(data) || data.size() > static_cast<qsizetype>(std::numeric_limits<uInt>::max())) {
        return {};
    }

    z_stream stream{};
    if (inflateInit2(&stream, kGzipWindowBits) != Z_OK) {
        return {};
    }

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.constData()));
    stream.avail_in = static_cast<uInt>(data.size());

    QByteArray output;
    std::array<char, kChunkSize> buffer{};
    int status = Z_OK;
    while (status == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer.data());
        stream.avail_out = static_cast<uInt>(buffer.size());
        status = inflate(&stream, Z_NO_FLUSH);
        output.append(buffer.data(), static_cast<qsizetype>(buffer.size() - stream.avail_out));
    }

    inflateEnd(&stream);
    return status == Z_STREAM_END ? output : QByteArray{};
}

bool GZipCompressor::isCompressed(const QByteArray &data) const
{
    return hasGzipHeader(data);
}

QByteArray GZipCompressor::peek(const QByteArray &data, int count) const
{
    if (count <= 0 || !hasGzipHeader(data) ||
        data.size() > static_cast<qsizetype>(std::numeric_limits<uInt>::max())) {
        return {};
    }

    z_stream stream{};
    if (inflateInit2(&stream, kGzipWindowBits) != Z_OK) {
        return {};
    }

    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.constData()));
    stream.avail_in = static_cast<uInt>(data.size());

    QByteArray output;
    output.resize(count);
    stream.next_out = reinterpret_cast<Bytef *>(output.data());
    stream.avail_out = static_cast<uInt>(count);

    const int status = inflate(&stream, Z_NO_FLUSH);
    const qsizetype produced = count - static_cast<int>(stream.avail_out);
    inflateEnd(&stream);

    if (status != Z_OK && status != Z_STREAM_END && status != Z_BUF_ERROR) {
        return {};
    }
    output.resize(produced);
    return output;
}

}  // namespace seb::configuration::data_compression
