#pragma once

#include "client_context.h"

#include <memory>

namespace seb::browser {
class BrowserApplication;
}
class SebSession;

namespace seb::client {

class ClientController;

class CompositionRoot
{
public:
    explicit CompositionRoot(SebSession &session);

    ClientController &controller();

private:
    SebSession &session_;
    ClientContext context_;
    std::shared_ptr<seb::browser::BrowserApplication> browser_;
    std::unique_ptr<ClientController> controller_;
};

}  // namespace seb::client
