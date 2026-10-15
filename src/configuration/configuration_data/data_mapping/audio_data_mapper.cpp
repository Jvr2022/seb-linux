#include "audio_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void AudioDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    if (key == keys::Audio::InitialVolumeLevel) settings.Audio.InitialVolume = helpers::integer(value, settings.Audio.InitialVolume);
    else if (key == keys::Audio::SetInitialVolumeLevel) settings.Audio.InitializeVolume = helpers::boolean(value, settings.Audio.InitializeVolume);
    else if (key == keys::Audio::MuteAudio) settings.Audio.MuteAudio = helpers::boolean(value, settings.Audio.MuteAudio);
}
}  // namespace seb::configuration::configuration_data::data_mapping
