#pragma once

#include "ImageFormat.h"
#include "ImageQuantization.h"
#include "MetaDataSettings.h"

#include <QString>

namespace seb::settings::proctoring {

struct ScreenProctoringSettings
{
    int CacheSize = 0;
    QString ClientId;
    QString ClientSecret;
    bool Enabled = false;
    QString EncryptionSecret;
    QString GroupId;
    seb::settings::proctoring::ImageFormat ImageFormat = seb::settings::proctoring::ImageFormat::Jpeg;
    double ImageDownscaling = 0.0;
    int IntervalMaximum = 0;
    int IntervalMinimum = 0;
    MetaDataSettings MetaData;
    ImageQuantization Quantization = ImageQuantization::Medium;
    QString ServiceUrl;
};

}  // namespace seb::settings::proctoring
