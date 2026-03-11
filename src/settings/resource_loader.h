#pragma once

#include "../seb_settings.h"

namespace seb::settingsinternal {

ResourceLoadResult loadSettingsFromNetworkResource(
    const QUrl &url,
    const std::function<QString (bool)> &passwordProvider);

}  // namespace seb::settingsinternal
