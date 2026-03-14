#pragma once

#include "events/icon_changed_event_handler.h"
#include "events/title_changed_event_handler.h"

#include <QString>

namespace seb::applications::contracts {

class IApplicationWindow
{
public:
    virtual ~IApplicationWindow() = default;

    virtual QString handle() const = 0;
    virtual QString iconPath() const = 0;
    virtual QString title() const = 0;
    virtual void activate() = 0;
};

}  // namespace seb::applications::contracts
