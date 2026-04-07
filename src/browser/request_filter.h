#pragma once

#include "webengine_compat.h"
#include "../seb_settings.h"

#include <QRegularExpression>
#include <QVector>
#if SEB_HAS_QTWEBENGINE
#include <QWebEngineUrlRequestInfo>
#endif

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

#if SEB_HAS_QTWEBENGINE
    FilterDecision evaluate(const QUrl &url, QWebEngineUrlRequestInfo::ResourceType resourceType) const;
#endif

private:
    struct CompiledRule
    {
        QRegularExpression expression;
        seb::FilterResult result = seb::FilterResult::Block;
    };

    static QRegularExpression compileRule(const seb::FilterRuleSettings &rule);
#if SEB_HAS_QTWEBENGINE
    static bool isMainRequest(QWebEngineUrlRequestInfo::ResourceType resourceType);
#endif

    seb::FilterSettings settings_;
    QVector<CompiledRule> allowRules_;
    QVector<CompiledRule> blockRules_;
};

}  // namespace seb::browser
