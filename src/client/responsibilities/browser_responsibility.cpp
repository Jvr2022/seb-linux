#include "browser_responsibility.h"

namespace seb::client::responsibilities {

BrowserResponsibility::BrowserResponsibility(ClientContext &context)
    : ClientResponsibility(context)
{
}

void BrowserResponsibility::assume(ClientTask task)
{
    if (!context_.browser) {
        return;
    }

    switch (task) {
    case ClientTask::AutoStartApplications:
        if (context_.browser->autoStart()) {
            context_.browser->start();
        }
        break;
    case ClientTask::RegisterEvents:
    case ClientTask::DeregisterEvents:
    case ClientTask::PrepareShutdownWave1:
    case ClientTask::PrepareShutdownWave2:
    case ClientTask::ScheduleIntegrityVerification:
    case ClientTask::ShowShell:
    case ClientTask::StartMonitoring:
    case ClientTask::UpdateSessionIntegrity:
    case ClientTask::VerifySessionIntegrity:
    case ClientTask::CloseShell:
        break;
    }
}

}  // namespace seb::client::responsibilities
