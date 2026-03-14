#pragma once

#include "../applications/contracts/application.h"
#include "../browser/contracts/IBrowserApplication.h"

#include <QList>
class SebSession;

namespace seb::client {
namespace responsibilities {
class ClientResponsibility;
}

struct ClientContext
{
    QList<seb::applications::contracts::IApplication *> applications;
    std::shared_ptr<seb::browser::contracts::IBrowserApplication> browser;
    bool quitPasswordValidated = false;
    QList<responsibilities::ClientResponsibility *> responsibilities;
    SebSession *session = nullptr;
};

}  // namespace seb::client
