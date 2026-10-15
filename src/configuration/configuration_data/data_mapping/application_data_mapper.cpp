#include "application_data_mapper.h"

#include "mapping_helpers.h"
#include "../keys.h"

namespace seb::configuration::configuration_data::data_mapping {

void ApplicationDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    if (key != keys::Applications::Blacklist && key != keys::Applications::Whitelist) return;
    if (!isList(value)) return;
    for (const QVariant &entry : value.toList()) {
        if (!isMap(entry)) continue;
        const QVariantMap raw = entry.toMap();
        const bool isWindowsProcess = isInteger(raw.value(keys::Applications::OperatingSystem)) &&
                                      integer(raw.value(keys::Applications::OperatingSystem)) == keys::WINDOWS;
        if (!isWindowsProcess) continue;

        if (key == keys::Applications::Blacklist) {
            const bool isActive = isBoolean(raw.value(keys::Applications::Active)) &&
                                  boolean(raw.value(keys::Applications::Active));
            if (!isActive || !isWindowsProcess) continue;

            seb::settings::applications::BlacklistApplication application;
            application.AutoTerminate = boolean(raw.value(keys::Applications::AutoTerminate), application.AutoTerminate);
            application.ExecutableName = string(raw.value(keys::Applications::ExecutableName));
            application.OriginalName = string(raw.value(keys::Applications::OriginalName));
            settings.Applications.Blacklist.push_back(application);
        } else {
            const bool isActive = isBoolean(raw.value(keys::Applications::Active)) &&
                                  boolean(raw.value(keys::Applications::Active));
            if (!isActive || !isWindowsProcess) continue;

            seb::settings::applications::WhitelistApplication application;
            application.AllowCustomPath = boolean(raw.value(keys::Applications::AllowCustomPath), application.AllowCustomPath);
            application.AllowRunning = boolean(raw.value(keys::Applications::AllowRunning), application.AllowRunning);
            const QVariant argumentsValue = raw.value(keys::Applications::Arguments);
            for (const QVariant &argumentEntry : (isList(argumentsValue) ? argumentsValue.toList() : QVariantList{})) {
                if (!isMap(argumentEntry)) continue;
                const QVariantMap argument = argumentEntry.toMap();
                if (isBoolean(argument.value(keys::Applications::Active)) &&
                    boolean(argument.value(keys::Applications::Active)) &&
                    isString(argument.value(keys::Applications::Argument))) {
                    application.Arguments.push_back(argument.value(keys::Applications::Argument).toString());
                }
            }
            application.AutoStart = boolean(raw.value(keys::Applications::AutoStart), application.AutoStart);
            application.AutoTerminate = boolean(raw.value(keys::Applications::AutoTerminate), application.AutoTerminate);
            application.Description = string(raw.value(keys::Applications::Description));
            application.DisplayName = string(raw.value(keys::Applications::DisplayName));
            application.ExecutableName = string(raw.value(keys::Applications::ExecutableName));
            application.ExecutablePath = string(raw.value(keys::Applications::ExecutablePath));
            application.OriginalName = string(raw.value(keys::Applications::OriginalName));
            application.ShowInShell = boolean(raw.value(keys::Applications::ShowInShell), application.ShowInShell);
            application.Signature = string(raw.value(keys::Applications::Signature));
            settings.Applications.Whitelist.push_back(application);
        }
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping
