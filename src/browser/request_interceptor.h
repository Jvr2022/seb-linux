#pragma once

#include "../settings/Browser/BrowserSettings.h"
#include "../configuration/contracts/cryptography/i_key_generator.h"
#include "request_filter.h"

#include "contracts/i_request_interceptor.h"

namespace seb::browser {

class RequestInterceptor : public contracts::IRequestInterceptor
{
public:
    RequestInterceptor(
        const seb::settings::browser::BrowserSettings &settings,
        seb::configuration::contracts::cryptography::IKeyGenerator &keyGenerator);

    void interceptRequest(contracts::IRequest &request) override;
    FilterDecision evaluate(
        const QUrl &url,
        contracts::ResourceType resourceType) const;

private:
    bool shouldAppendHeaders(const contracts::IRequest &request) const;
    static bool sameHost(const QUrl &lhs, const QUrl &rhs);

    seb::settings::browser::BrowserSettings settings_;
    RequestFilter filter_;
    seb::configuration::contracts::cryptography::IKeyGenerator &keyGenerator_;
};

}  // namespace seb::browser
