#pragma once

#include "client_responsibility.h"

namespace seb::client::responsibilities {

class BrowserResponsibility : public ClientResponsibility
{
public:
    explicit BrowserResponsibility(ClientContext &context);

    void assume(ClientTask task) override;
};

}  // namespace seb::client::responsibilities
