#pragma once

#include "webengine_compat.h"
#include "../seb_settings.h"
#include "key_generator.h"
#include "request_filter.h"

#if SEB_HAS_QTWEBENGINE
#include <QWebEngineUrlRequestInterceptor>
#else
#include <QObject>
#endif

namespace seb::browser {

#if SEB_HAS_QTWEBENGINE
class RequestInterceptor : public QWebEngineUrlRequestInterceptor
{
public:
    explicit RequestInterceptor(const seb::SebSettings &settings, QObject *parent = nullptr);

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

private:
    bool shouldAppendHeaders(const QWebEngineUrlRequestInfo &info) const;
    static bool sameHost(const QUrl &lhs, const QUrl &rhs);
    static QUrl replaceSebScheme(const QUrl &url);

    seb::SebSettings settings_;
    RequestFilter filter_;
    KeyGenerator keyGenerator_;
};
#else
class RequestInterceptor : public QObject
{
public:
    explicit RequestInterceptor(const seb::SebSettings &settings, QObject *parent = nullptr);
};
#endif

}  // namespace seb::browser
