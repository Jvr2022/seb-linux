#pragma once

#include <QString>

namespace seb::browser::integrations {

class Integration
{
public:
    virtual ~Integration() = default;
    virtual QString name() const = 0;
};

}  // namespace seb::browser::integrations
