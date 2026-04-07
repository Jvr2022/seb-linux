#include "webkitgtk_provider.h"

#if !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
#include "webkitgtk_profile.h"
#include "webkitgtk_view.h"
#include "../../contracts/i_webprofile.h"
#include "../../contracts/i_webview.h"
#include <memory>
#include <QObject>
#include <QWidget>

namespace seb::browser {

std::unique_ptr<contracts::IWebProfile> WebKitGtkProvider::createProfile(QObject *parent)
{
    return std::make_unique<WebKitGtkProfile>(parent);
}

std::unique_ptr<contracts::IWebView> WebKitGtkProvider::createWebView(contracts::IWebProfile *profile, QWidget *parent)
{
    auto *webkitProfile = static_cast<WebKitGtkProfile *>(profile);
    return std::make_unique<WebKitGtkWebView>(webkitProfile, parent);
}

}  // namespace seb::browser

#endif // !SEB_HAS_QTWEBENGINE && SEB_HAS_WEBKITGTK
