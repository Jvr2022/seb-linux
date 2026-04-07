#pragma once

#if SEB_HAS_QTWEBENGINE

#include "browser/contracts/i_engine_provider.h"

namespace seb::browser {

class QtWebEngineProvider : public contracts::IEngineProvider
{
public:
    QtWebEngineProvider() = default;
    ~QtWebEngineProvider() override = default;

    std::unique_ptr<contracts::IWebProfile> createProfile(QObject *parent) override;
    std::unique_ptr<contracts::IWebView> createWebView(contracts::IWebProfile *profile, QWidget *parent) override;
};

}  // namespace seb::browser

#endif
