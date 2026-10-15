#pragma once

#include "../../settings/AppSettings.h"

#include <QVariantMap>

namespace seb::configuration::configuration_data {

class DataProcessor
{
public:
    void processDefault(seb::settings::AppSettings &settings) const;
    void process(const QVariantMap &data, seb::settings::AppSettings &settings) const;

private:
    void allowBrowserToolbarForReloading(seb::settings::AppSettings &settings) const;
    void calculateConfigurationKey(const QVariantMap &data, seb::settings::AppSettings &settings) const;
    void initializeBrowserHomeFunctionality(seb::settings::AppSettings &settings) const;
    void initializeClipboardSettings(seb::settings::AppSettings &settings) const;
    void initializeProctoringSettings(seb::settings::AppSettings &settings) const;
    void removeLegacyBrowsers(seb::settings::AppSettings &settings) const;
};

}  // namespace seb::configuration::configuration_data
