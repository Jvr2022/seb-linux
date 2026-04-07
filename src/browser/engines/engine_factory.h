#pragma once

#include "../contracts/i_engine_provider.h"
#include <memory>

namespace seb::browser {

/*!
 * \brief Returns the active engine provider based on build-time configuration.
 * On complete builds this returns QtWebEngineProvider. On compatibility builds
 * this returns WebKitGtkProvider.
 */
std::unique_ptr<contracts::IEngineProvider> createEngineProvider();

}  // namespace seb::browser
