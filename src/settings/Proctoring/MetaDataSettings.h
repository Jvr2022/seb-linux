#pragma once
namespace seb::settings::proctoring {
struct MetaDataSettings {
    bool CaptureApplicationData = false;
    bool CaptureAppForeground = false;
    bool CaptureBrowserData = false;
    bool CaptureBrowserInfo = false;
    bool CaptureScreen = false;
    bool CaptureWindowTitle = false;
};
}
