#pragma once
#include "../settings/Browser/FilterSettings.h"

#include <QRegularExpression>
#include <QVector>

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
    struct CompiledRule
    {
        QRegularExpression expression;
        seb::settings::browser::filter::FilterResult result =
            seb::settings::browser::filter::FilterResult::Block;
    };

    static QRegularExpression compileRule(
        const seb::settings::browser::filter::FilterRuleSettings &rule);
    static bool isMainRequest(contracts::ResourceType resourceType);

    seb::settings::browser::FilterSettings settings_;
    QVector<CompiledRule> allowRules_;
    QVector<CompiledRule> blockRules_;
};

}  // namespace seb::browser
