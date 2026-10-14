#include "proctoring_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ProctoringDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &p = settings.Proctoring;
    auto &screen = p.ScreenProctoring;
    if (key == keys::Proctoring::ClientId) { p.ClientId = string(value); screen.ClientId = p.ClientId; }
    else if (key == keys::Proctoring::ClientSecret) { p.ClientSecret = string(value); screen.ClientSecret = p.ClientSecret; }
    else if (key == keys::Proctoring::Enabled) p.Enabled = boolean(value, p.Enabled);
    else if (key == keys::Proctoring::ServiceUrl) { p.ServiceUrl = string(value); screen.ServiceUrl = p.ServiceUrl; }
    else if (key == keys::Proctoring::ScreenCacheSize) screen.CacheSize = integer(value, screen.CacheSize);
    else if (key == keys::Proctoring::ScreenEnabled) { screen.Enabled = boolean(value, screen.Enabled); p.Enabled = screen.Enabled; }
    else if (key == keys::Proctoring::ScreenGroupId) screen.GroupId = string(value);
    else if (key == keys::Proctoring::ScreenImageDownscaling) screen.ImageDownscaling = real(value, screen.ImageDownscaling);
    else if (key == keys::Proctoring::ScreenImageFormat) screen.ImageFormat = normalized(value) == QStringLiteral("png") ? seb::settings::proctoring::ImageFormat::Png : seb::settings::proctoring::ImageFormat::Jpeg;
    else if (key == keys::Proctoring::ScreenImageQuantization) screen.Quantization = static_cast<seb::settings::proctoring::ImageQuantization>(integer(value, static_cast<int>(screen.Quantization)));
    else if (key == keys::Proctoring::ScreenIntervalMaximum) screen.IntervalMaximum = integer(value, screen.IntervalMaximum);
    else if (key == keys::Proctoring::ScreenIntervalMinimum) screen.IntervalMinimum = integer(value, screen.IntervalMinimum);
    else if (key == keys::Proctoring::ScreenShowDisclaimer) { p.ShowDisclaimer = boolean(value, p.ShowDisclaimer); screen.ShowDisclaimer = p.ShowDisclaimer; }
    else if (key == keys::Proctoring::ScreenShowTaskbarNotification) { p.ShowTaskbarNotification = boolean(value, p.ShowTaskbarNotification); screen.ShowTaskbarNotification = p.ShowTaskbarNotification; }
    else if (key == keys::Proctoring::CaptureApplicationData) { p.MetaData.CaptureApplicationData = boolean(value, p.MetaData.CaptureApplicationData); screen.MetaData.CaptureApplicationData = p.MetaData.CaptureApplicationData; }
    else if (key == keys::Proctoring::CaptureBrowserData) { p.MetaData.CaptureBrowserData = boolean(value, p.MetaData.CaptureBrowserData); screen.MetaData.CaptureBrowserData = p.MetaData.CaptureBrowserData; }
    else if (key == keys::Proctoring::CaptureWindowTitle) { p.MetaData.CaptureWindowTitle = boolean(value, p.MetaData.CaptureWindowTitle); screen.MetaData.CaptureWindowTitle = p.MetaData.CaptureWindowTitle; }
}
}  // namespace seb::configuration::configuration_data::data_mapping
