#pragma once

#include "i_webprofile.h"
#include "i_webview.h"

#include <memory>

QT_BEGIN_NAMESPACE
class QObject;
class QWidget;
QT_END_NAMESPACE

namespace seb::browser {
class RequestInterceptor;
}

namespace seb::browser::contracts {

class IEngineProvider
{
public:
    virtual ~IEngineProvider() = default;

    virtual std::unique_ptr<IWebProfile> createProfile(QObject *parent) = 0;
    virtual std::unique_ptr<IWebView> createWebView(IWebProfile *profile, QWidget *parent) = 0;
};

}  // namespace seb::browser::contracts
