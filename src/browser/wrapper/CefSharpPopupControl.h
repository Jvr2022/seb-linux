#pragma once

#include "ICefSharpControl.h"

namespace seb::browser::wrapper {

class CefSharpPopupControl : public ICefSharpControl
{
public:
    QString address() const override { return {}; }
};

}  // namespace seb::browser::wrapper
