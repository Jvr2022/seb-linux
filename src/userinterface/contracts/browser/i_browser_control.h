#pragma once

#include "events/address_changed_event_handler.h"
#include "events/find_requested_event_handler.h"
#include "events/load_failed_event_handler.h"
#include "events/loading_state_changed_event_handler.h"
#include "events/title_changed_event_handler.h"

#include <QString>
#include <QUrl>

namespace seb::userinterface::contracts::browser {

class IBrowserControl
{
public:
    virtual ~IBrowserControl() = default;
    virtual QString address() const = 0;
    virtual bool isLoading() const = 0;
    virtual QString title() const = 0;
    virtual void findText(const QString &text, bool forward = true, bool matchCase = false) = 0;
    virtual void load(const QUrl &url) = 0;
};

}  // namespace seb::userinterface::contracts::browser
