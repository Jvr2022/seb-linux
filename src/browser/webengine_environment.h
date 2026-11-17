#pragma once

#include "../settings/Browser/BrowserSettings.h"

namespace seb::browser {

bool applyWebEngineEnvironment(const seb::settings::browser::BrowserSettings &settings, QString *error = nullptr);
void freezeWebEngineEnvironment();

}  // namespace seb::browser
