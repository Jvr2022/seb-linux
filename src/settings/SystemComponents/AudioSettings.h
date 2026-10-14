#pragma once
namespace seb::settings::systemcomponents {
struct AudioSettings {
    bool EnableVolumeControl = true;
    bool InitializeVolume = false;
    int InitialVolume = 100;
    bool MuteAudio = false;
};
}
