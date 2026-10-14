#pragma once
namespace seb::settings::server {
struct InvigilationSettings {
    bool EnableRemoteProctoring = false;
    bool ForceRaiseHandMessage = false;
    bool ShowRaiseHandNotification = true;
};
}
