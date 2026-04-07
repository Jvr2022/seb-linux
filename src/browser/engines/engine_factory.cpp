#include "engine_factory.h"
#include "../webengine_compat.h"

#if SEB_HAS_QTWEBENGINE
#include "qtwebengine/qt_webengine_provider.h"
#elif SEB_HAS_WEBKITGTK
#include "webkitgtk/webkitgtk_provider.h"
#endif

namespace seb::browser {

std::unique_ptr<contracts::IEngineProvider> createEngineProvider()
{
#if SEB_HAS_QTWEBENGINE
    return std::make_unique<QtWebEngineProvider>();
#elif SEB_HAS_WEBKITGTK
    return std::make_unique<WebKitGtkProvider>();
#else
    return nullptr;
#endif
}

}  // namespace seb::browser
