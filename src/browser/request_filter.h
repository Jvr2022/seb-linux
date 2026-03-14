#pragma once

#include "../seb_settings.h"

#include <QRegularExpression>
#include <QVector>
#include <QWebEngineUrlRequestInfo>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

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
    explicit RequestFilter(const seb::FilterSettings &settings);

    FilterDecision evaluate(const QUrl &url, QWebEngineUrlRequestInfo::ResourceType resourceType) const;

private:
    struct CompiledRule
    {
        QRegularExpression expression;
        seb::FilterResult result = seb::FilterResult::Block;
    };

    static QRegularExpression compileRule(const seb::FilterRuleSettings &rule);
    static bool isMainRequest(QWebEngineUrlRequestInfo::ResourceType resourceType);

    seb::FilterSettings settings_;
    QVector<CompiledRule> allowRules_;
    QVector<CompiledRule> blockRules_;
};

}  // namespace seb::browser
