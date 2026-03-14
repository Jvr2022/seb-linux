#pragma once

#include "application_window.h"
#include "events/windows_changed_event_handler.h"

#include <QList>
#include <QString>

namespace seb::applications::contracts {

class IApplication
{
public:
    virtual ~IApplication() = default;

    virtual bool autoStart() const = 0;
    virtual QString iconPath() const = 0;
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString tooltip() const = 0;
    virtual QList<IApplicationWindow *> windows() const = 0;
    virtual void initialize() = 0;
    virtual void start() = 0;
    virtual void terminate() = 0;
};

}  // namespace seb::applications::contracts
