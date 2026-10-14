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
            seb::settings::applications::BlacklistApplication application;
            application.AutoTerminate = boolean(raw.value(QStringLiteral("strongKill")), boolean(raw.value(QStringLiteral("autoTerminate"))));
            application.ExecutableName = string(raw.value(QStringLiteral("executable")));
            if (application.ExecutableName.isEmpty()) application.ExecutableName = string(raw.value(QStringLiteral("executableName")));
            application.OperatingSystem = integer(raw.value(QStringLiteral("os")));
            application.OriginalName = string(raw.value(QStringLiteral("originalName")));
            settings.Applications.Blacklist.push_back(application);
        } else {
            seb::settings::applications::WhitelistApplication application;
            application.AllowCustomPath = boolean(raw.value(QStringLiteral("allowUserToChooseApp")), boolean(raw.value(QStringLiteral("allowCustomPath"))));
            application.AllowRunning = boolean(raw.value(QStringLiteral("runInBackground")), boolean(raw.value(QStringLiteral("allowRunning"))));
            application.Arguments = strings(raw.value(QStringLiteral("arguments")));
            application.AutoStart = boolean(raw.value(QStringLiteral("autostart")), boolean(raw.value(QStringLiteral("autoStart"))));
            application.AutoTerminate = boolean(raw.value(QStringLiteral("strongKill")), boolean(raw.value(QStringLiteral("autoTerminate"))));
            application.Description = string(raw.value(QStringLiteral("description")));
            application.DisplayName = string(raw.value(QStringLiteral("title")));
            if (application.DisplayName.isEmpty()) application.DisplayName = string(raw.value(QStringLiteral("displayName")));
            application.ExecutableName = string(raw.value(QStringLiteral("executable")));
            if (application.ExecutableName.isEmpty()) application.ExecutableName = string(raw.value(QStringLiteral("executableName")));
            application.ExecutablePath = string(raw.value(QStringLiteral("path")));
            if (application.ExecutablePath.isEmpty()) application.ExecutablePath = string(raw.value(QStringLiteral("executablePath")));
            application.Identifier = string(raw.value(QStringLiteral("identifier")));
            application.OperatingSystem = integer(raw.value(QStringLiteral("os")));
            application.OriginalName = string(raw.value(QStringLiteral("originalName")));
            application.ShowInShell = boolean(raw.value(QStringLiteral("iconInTaskbar")), boolean(raw.value(QStringLiteral("showInShell")), true));
            application.Signature = string(raw.value(QStringLiteral("signature")));
            settings.Applications.Whitelist.push_back(application);
        }
    }
}

}  // namespace seb::configuration::configuration_data::data_mapping
