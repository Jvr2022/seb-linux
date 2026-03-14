#include "client_controller.h"

#include "responsibilities/client_responsibility.h"
#include "responsibilities/client_task.h"

namespace seb::client {

ClientController::ClientController(ClientContext &context, QObject *parent)
    : QObject(parent)
    , context_(context)
    , operations_(this)
{
}

operations::ClientOperationSequence &ClientController::operations()
{
    return operations_;
}

bool ClientController::tryStart()
{
    const auto result = operations_.perform();
    if (result != operations::OperationResult::Success) {
        return false;
    }

    delegateStartupResponsibilities();
    return true;
}

void ClientController::terminate()
{
    delegateShutdownResponsibilities();
    operations_.revert();
}

void ClientController::delegateStartupResponsibilities()
{
    for (const auto &responsibility : context_.responsibilities) {
        if (!responsibility) {
            continue;
        }
        responsibility->assume(responsibilities::ClientTask::RegisterEvents);
        responsibility->assume(responsibilities::ClientTask::ShowShell);
        responsibility->assume(responsibilities::ClientTask::AutoStartApplications);
    }
}

void ClientController::delegateShutdownResponsibilities()
{
    for (const auto &responsibility : context_.responsibilities) {
        if (!responsibility) {
            continue;
        }
        responsibility->assume(responsibilities::ClientTask::CloseShell);
        responsibility->assume(responsibilities::ClientTask::DeregisterEvents);
    }
}

}  // namespace seb::client
