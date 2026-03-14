#pragma once

#include "ImageFormat.h"
#include "ImageQuantization.h"

namespace seb::settings::proctoring {

struct ScreenProctoringSettings
{
    bool Enabled = false;
    ImageFormat ImageFormat = ImageFormat::Jpeg;
    ImageQuantization Quantization = ImageQuantization::Medium;
};

}  // namespace seb::settings::proctoring
