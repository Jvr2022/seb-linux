#include "proctoring_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ProctoringDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &p = settings.Proctoring;
    auto &screen = p.ScreenProctoring;
    if (key == keys::Proctoring::ScreenProctoring::ClientId) screen.ClientId = string(value, screen.ClientId);
    else if (key == keys::Proctoring::ScreenProctoring::ClientSecret) screen.ClientSecret = string(value, screen.ClientSecret);
    else if (key == keys::Proctoring::ScreenProctoring::ServiceUrl) screen.ServiceUrl = string(value, screen.ServiceUrl);
    else if (key == keys::Proctoring::ScreenProctoring::CacheSize) screen.CacheSize = integer(value, screen.CacheSize);
    else if (key == keys::Proctoring::ScreenProctoring::Enabled) screen.Enabled = boolean(value, screen.Enabled);
    else if (key == keys::Proctoring::ScreenProctoring::GroupId) screen.GroupId = string(value, screen.GroupId);
    else if (key == keys::Proctoring::ScreenProctoring::ImageDownscaling) screen.ImageDownscaling = real(value, screen.ImageDownscaling);
    else if (key == keys::Proctoring::ScreenProctoring::ImageFormat && isString(value)) screen.ImageFormat = normalized(value) == QStringLiteral("png") ? seb::settings::proctoring::ImageFormat::Png : seb::settings::proctoring::ImageFormat::Jpeg;
    else if (key == keys::Proctoring::ScreenProctoring::ImageQuantization) screen.Quantization = static_cast<seb::settings::proctoring::ImageQuantization>(integer(value, static_cast<int>(screen.Quantization)));
    else if (key == keys::Proctoring::ScreenProctoring::IntervalMaximum) screen.IntervalMaximum = integer(value, screen.IntervalMaximum);
    else if (key == keys::Proctoring::ScreenProctoring::IntervalMinimum) screen.IntervalMinimum = integer(value, screen.IntervalMinimum);
    else if (key == keys::Proctoring::ShowDisclaimer) p.ShowDisclaimer = boolean(value, p.ShowDisclaimer);
    else if (key == keys::Proctoring::ShowTaskbarNotification) p.ShowTaskbarNotification = boolean(value, p.ShowTaskbarNotification);
    else if (key == keys::Proctoring::ScreenProctoring::MetaData::CaptureApplicationData) screen.MetaData.CaptureApplicationData = boolean(value, screen.MetaData.CaptureApplicationData);
    else if (key == keys::Proctoring::ScreenProctoring::MetaData::CaptureBrowserData) screen.MetaData.CaptureBrowserData = boolean(value, screen.MetaData.CaptureBrowserData);
    else if (key == keys::Proctoring::ScreenProctoring::MetaData::CaptureWindowTitle) screen.MetaData.CaptureWindowTitle = boolean(value, screen.MetaData.CaptureWindowTitle);
}
}  // namespace seb::configuration::configuration_data::data_mapping
