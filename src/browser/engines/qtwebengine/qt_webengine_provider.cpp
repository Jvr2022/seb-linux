#include "qt_webengine_provider.h"

#if SEB_HAS_QTWEBENGINE
#include "../../webengine_compat.h"
#include "qt_webengine_profile.h"
#include "qt_webengine_view.h"

namespace seb::browser {

std::unique_ptr<contracts::IWebProfile> QtWebEngineProvider::createProfile(QObject *parent)
{
    return std::make_unique<QtWebEngineProfile>(parent);
}

std::unique_ptr<contracts::IWebView> QtWebEngineProvider::createWebView(contracts::IWebProfile *profile, QWidget *parent)
{
    auto *qtProfile = static_cast<QtWebEngineProfile *>(profile);
    return std::make_unique<QtWebEngineWebView>(qtProfile, parent);
}

}  // namespace seb::browser

#endif
