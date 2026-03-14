#pragma once

#include "Filter/FilterRuleSettings.h"

#include <QList>

namespace seb::settings::browser {

struct FilterSettings
{
    bool ProcessContentRequests = false;
    bool ProcessMainRequests = false;
    QList<filter::FilterRuleSettings> Rules;
};

}  // namespace seb::settings::browser
