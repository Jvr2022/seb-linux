#pragma once
#include "../settings/Browser/FilterSettings.h"

#include "filters/rules/i_rule.h"

#include <memory>
#include <vector>

#include "contracts/i_request_interceptor.h"

#include <QtGlobal>
#include <QUrl>

namespace seb::browser {

enum class FilterDecision
{
    NoMatch,
    Allow,
    Block
};

class RequestFilter
{
public:
    explicit RequestFilter(const seb::settings::browser::FilterSettings &settings);

    FilterDecision evaluate(const QUrl &url, contracts::ResourceType resourceType) const;

private:
    struct LoadedRule
    {
        std::unique_ptr<filters::rules::IRule> rule;
        seb::settings::browser::filter::FilterResult result =
            seb::settings::browser::filter::FilterResult::Block;
    };

    static bool isMainRequest(contracts::ResourceType resourceType);

    seb::settings::browser::FilterSettings settings_;
    std::vector<LoadedRule> allowRules_;
    std::vector<LoadedRule> blockRules_;
};

}  // namespace seb::browser
