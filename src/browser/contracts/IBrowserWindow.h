#pragma once

#include "../../applications/contracts/application_window.h"

#include <QString>

namespace seb::browser::contracts {

class IBrowserWindow : public seb::applications::contracts::IApplicationWindow
{
public:
    ~IBrowserWindow() override = default;

    virtual bool isMainWindow() const = 0;
    virtual QString url() const = 0;
};

}  // namespace seb::browser::contracts
