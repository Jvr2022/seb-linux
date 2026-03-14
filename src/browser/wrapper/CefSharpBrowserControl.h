#pragma once

#include "ICefSharpControl.h"

namespace seb::browser::wrapper {

class CefSharpBrowserControl : public ICefSharpControl
{
public:
    QString address() const override { return {}; }
};

}  // namespace seb::browser::wrapper
