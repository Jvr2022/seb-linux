#pragma once

#include "../seb_settings.h"

#include <memory>

namespace seb::applications {

class ExternalApplication;

class ApplicationFactory
{
public:
    static std::unique_ptr<ExternalApplication> tryCreate(const seb::WhitelistApplicationSettings &settings);
};

}  // namespace seb::applications
