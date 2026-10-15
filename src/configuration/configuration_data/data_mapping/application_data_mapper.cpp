#include "application_data_mapper.h"

#include "mapping_helpers.h"
#include "../keys.h"

namespace seb::configuration::configuration_data::data_mapping {

void ApplicationDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    if (key != keys::Applications::Blacklist && key != keys::Applications::Whitelist) return;
    for (const QVariant &entry : value.toList()) {
        const QVariantMap raw = entry.toMap();
        if (key == keys::Applications::Blacklist) {
            const bool isActive = boolean(raw.value(keys::Applications::Active));
            const bool isWindowsProcess = integer(raw.value(keys::Applications::OperatingSystem)) == keys::WINDOWS;
            if (!isActive || !isWindowsProcess) continue;

            seb::settings::applications::BlacklistApplication application;
            application.AutoTerminate = boolean(raw.value(keys::Applications::AutoTerminate));
            application.ExecutableName = string(raw.value(keys::Applications::ExecutableName));
            application.OperatingSystem = integer(raw.value(keys::Applications::OperatingSystem));
            application.OriginalName = string(raw.value(keys::Applications::OriginalName));
            settings.Applications.Blacklist.push_back(application);
        } else {
            const bool isActive = boolean(raw.value(keys::Applications::Active));
            const bool isWindowsProcess = integer(raw.value(keys::Applications::OperatingSystem)) == keys::WINDOWS;
            if (!isActive || !isWindowsProcess) continue;

            seb::settings::applications::WhitelistApplication application;
            application.AllowCustomPath = boolean(raw.value(keys::Applications::AllowCustomPath));
            application.AllowRunning = boolean(raw.value(keys::Applications::AllowRunning));
            const QVariantList arguments = raw.value(keys::Applications::Arguments).toList();
            for (const QVariant &argumentEntry : arguments) {
                const QVariantMap argument = argumentEntry.toMap();
                if (boolean(argument.value(keys::Applications::Active))) {
                    const QString text = string(argument.value(keys::Applications::Argument));
                    if (!text.isEmpty()) application.Arguments.push_back(text);
                }
            }
            application.AutoStart = boolean(raw.value(keys::Applications::AutoStart));
            application.AutoTerminate = boolean(raw.value(keys::Applications::AutoTerminate));
            application.Description = string(raw.value(keys::Applications::Description));
            application.DisplayName = string(raw.value(keys::Applications::DisplayName));
            application.ExecutableName = string(raw.value(keys::Applications::ExecutableName));
            application.ExecutablePath = string(raw.value(keys::Applications::ExecutablePath));
            application.OperatingSystem = integer(raw.value(keys::Applications::OperatingSystem));
            application.OriginalName = string(raw.value(keys::Applications::OriginalName));
            application.ShowInShell = boolean(raw.value(keys::Applications::ShowInShell), application.ShowInShell);
            application.Signature = string(raw.value(keys::Applications::Signature));
            settings.Applications.Whitelist.push_back(application);
        }
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping
