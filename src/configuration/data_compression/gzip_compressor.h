#pragma once

#include "../contracts/data_compression/i_data_compressor.h"

namespace seb::configuration::data_compression {

class GZipCompressor : public contracts::data_compression::IDataCompressor
{
public:
    QByteArray compress(const QByteArray &data) const override;
    QByteArray decompress(const QByteArray &data) const override;
    bool isCompressed(const QByteArray &data) const override;
    QByteArray peek(const QByteArray &data, int count) const override;
};

}  // namespace seb::configuration::data_compression
