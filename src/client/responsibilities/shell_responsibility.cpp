#include "shell_responsibility.h"

#include "../../seb_session.h"

namespace seb::client::responsibilities {

ShellResponsibility::ShellResponsibility(ClientContext &context)
    : ClientResponsibility(context)
{
}

void ShellResponsibility::assume(ClientTask task)
{
    if (!context_.session) {
        return;
    }

    switch (task) {
    case ClientTask::CloseShell:
    case ClientTask::DeregisterEvents:
    case ClientTask::PrepareShutdownWave1:
    case ClientTask::PrepareShutdownWave2:
    case ClientTask::RegisterEvents:
    case ClientTask::ScheduleIntegrityVerification:
    case ClientTask::ShowShell:
    case ClientTask::StartMonitoring:
    case ClientTask::UpdateSessionIntegrity:
    case ClientTask::VerifySessionIntegrity:
    case ClientTask::AutoStartApplications:
        break;
    }
}

}  // namespace seb::client::responsibilities
