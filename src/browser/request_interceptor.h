#pragma once

#include "../seb_settings.h"
#include "key_generator.h"
#include "request_filter.h"

#include "contracts/i_request_interceptor.h"

namespace seb::browser {

class RequestInterceptor : public contracts::IRequestInterceptor
{
public:
    explicit RequestInterceptor(const seb::SebSettings &settings);

    void interceptRequest(contracts::IRequest &request) override;

private:
    bool shouldAppendHeaders(const contracts::IRequest &request) const;
    static bool sameHost(const QUrl &lhs, const QUrl &rhs);
    static QUrl replaceSebScheme(const QUrl &url);

    seb::SebSettings settings_;
    RequestFilter filter_;
    KeyGenerator keyGenerator_;
};

}  // namespace seb::browser
