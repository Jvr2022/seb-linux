#pragma once

namespace seb::client::responsibilities {

enum class ClientTask
{
    AutoStartApplications,
    CloseShell,
    DeregisterEvents,
    PrepareShutdownWave1,
    PrepareShutdownWave2,
    RegisterEvents,
    ScheduleIntegrityVerification,
    ShowShell,
    StartMonitoring,
    UpdateSessionIntegrity,
    VerifySessionIntegrity
};

}  // namespace seb::client::responsibilities
