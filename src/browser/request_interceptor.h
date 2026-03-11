#pragma once

#include "../seb_settings.h"
#include "key_generator.h"
#include "request_filter.h"

#include <QWebEngineUrlRequestInterceptor>

namespace seb::browser {

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

}  // namespace seb::browser
