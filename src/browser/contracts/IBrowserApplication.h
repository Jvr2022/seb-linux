#pragma once

#include "../../applications/contracts/application.h"
#include "IBrowserWindow.h"

namespace seb::browser::contracts {

class IBrowserApplication : public seb::applications::contracts::IApplication
{
public:
    ~IBrowserApplication() override = default;

    virtual void focus(bool forward) = 0;
    virtual QList<IBrowserWindow *> browserWindows() const = 0;
};

}  // namespace seb::browser::contracts
