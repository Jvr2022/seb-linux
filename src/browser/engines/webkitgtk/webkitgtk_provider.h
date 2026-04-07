#pragma once

#if !SEB_HAS_QTWEBENGINE

#include <memory>
#include <QObject>
#include <QWidget>

#include "../../contracts/i_engine_provider.h"

namespace seb::browser {

class WebKitGtkProvider : public contracts::IEngineProvider
{
public:
    WebKitGtkProvider() = default;
    ~WebKitGtkProvider() override = default;

    std::unique_ptr<contracts::IWebProfile> createProfile(QObject *parent) override;
    std::unique_ptr<contracts::IWebView> createWebView(contracts::IWebProfile *profile, QWidget *parent) override;
};

}  // namespace seb::browser

#endif
