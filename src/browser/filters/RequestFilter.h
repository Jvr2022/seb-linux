#pragma once

#include "../../seb_settings.h"

#include "RuleFactory.h"

#include <QString>
#include <QVector>

namespace seb::browser::filters {

class RequestFilter
{
public:
    explicit RequestFilter(const seb::FilterSettings &settings);

    bool isAllowed(const QString &url) const;

private:
    struct CompiledRule
    {
        RuleVariant rule;
        seb::FilterResult result = seb::FilterResult::Block;
    };

    QVector<CompiledRule> allowRules_;
    QVector<CompiledRule> blockRules_;
    seb::FilterResult defaultResult_ = seb::FilterResult::Block;
};

}  // namespace seb::browser::filters
