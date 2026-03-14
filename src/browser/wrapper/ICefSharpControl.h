#pragma once

#include <QString>

namespace seb::browser::wrapper {

class ICefSharpControl
{
public:
    virtual ~ICefSharpControl() = default;
    virtual QString address() const = 0;
};

}  // namespace seb::browser::wrapper
