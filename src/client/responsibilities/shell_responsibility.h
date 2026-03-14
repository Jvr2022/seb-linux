#pragma once

#include "client_responsibility.h"

namespace seb::client::responsibilities {

class ShellResponsibility : public ClientResponsibility
{
public:
    explicit ShellResponsibility(ClientContext &context);

    void assume(ClientTask task) override;
};

}  // namespace seb::client::responsibilities
