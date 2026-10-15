#pragma once

#include "../settings/Applications/WhitelistApplication.h"

#include <memory>

namespace seb::applications {

class ExternalApplication;

class ApplicationFactory
{
public:
    static std::unique_ptr<ExternalApplication> tryCreate(const seb::settings::applications::WhitelistApplication &settings);
};

}  // namespace seb::applications
