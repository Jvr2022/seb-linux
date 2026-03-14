#pragma once

#include "../../seb_settings.h"
#include "factory_result.h"

namespace seb::applications::contracts {

class IApplication;

class IApplicationFactory
{
public:
    virtual ~IApplicationFactory() = default;
    virtual FactoryResult tryCreate(const seb::WhitelistApplicationSettings &settings, IApplication *&application) = 0;
};

}  // namespace seb::applications::contracts
