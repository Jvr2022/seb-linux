#pragma once

#include "../seb_settings.h"

namespace seb::settingsinternal {

SebSettings createDefaultSettings();
void finalizeSettings(SebSettings &settings);

}  // namespace seb::settingsinternal
