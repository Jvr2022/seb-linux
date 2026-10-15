#include "user_interface_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void UserInterfaceDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &ui = settings.UserInterface;
    if (key == keys::UserInterface::ActionCenter::EnableActionCenter) ui.ActionCenter.EnableActionCenter = boolean(value, ui.ActionCenter.EnableActionCenter);
    else if (key == keys::UserInterface::LockScreen::BackgroundColor) ui.LockScreen.BackgroundColor = string(value);
    else if (key == keys::UserInterface::Taskbar::EnableTaskbar) ui.Taskbar.EnableTaskbar = boolean(value, ui.Taskbar.EnableTaskbar);
    else if (key == keys::UserInterface::Taskbar::ShowVerificator) ui.Taskbar.ShowVerificator = boolean(value, ui.Taskbar.ShowVerificator);
    else if (key == keys::UserInterface::Taskbar::ShowApplicationLog) { ui.Taskbar.ShowApplicationLog = boolean(value, ui.Taskbar.ShowApplicationLog); ui.ActionCenter.ShowApplicationLog = ui.Taskbar.ShowApplicationLog; }
    else if (key == keys::UserInterface::SystemControls::Audio::Show) { ui.Taskbar.ShowAudio = boolean(value, ui.Taskbar.ShowAudio); ui.Taskbar.ShowAudioControl = ui.Taskbar.ShowAudio; ui.ActionCenter.ShowAudio = ui.Taskbar.ShowAudio; }
    else if (key == keys::UserInterface::SystemControls::Clock::Show) { ui.Taskbar.ShowClock = boolean(value, ui.Taskbar.ShowClock); ui.ActionCenter.ShowClock = ui.Taskbar.ShowClock; }
    else if (key == keys::UserInterface::SystemControls::KeyboardLayout::Show) { ui.Taskbar.ShowKeyboardLayout = boolean(value, ui.Taskbar.ShowKeyboardLayout); ui.ActionCenter.ShowKeyboardLayout = ui.Taskbar.ShowKeyboardLayout; }
    else if (key == keys::UserInterface::SystemControls::Network::Show) { ui.Taskbar.ShowNetwork = boolean(value, ui.Taskbar.ShowNetwork); ui.ActionCenter.ShowNetwork = ui.Taskbar.ShowNetwork; }
    else if (key == keys::UserInterface::UserInterfaceMode) ui.Mode = boolean(value) ? seb::settings::userinterface::UserInterfaceMode::Mobile : seb::settings::userinterface::UserInterfaceMode::Desktop;
    else if (key == keys::UserInterface::SystemControls::PowerSupply::ChargeThresholdCritical) settings.PowerSupply.ChargeThresholdCritical = real(value, settings.PowerSupply.ChargeThresholdCritical);
    else if (key == keys::UserInterface::SystemControls::PowerSupply::ChargeThresholdLow) settings.PowerSupply.ChargeThresholdLow = real(value, settings.PowerSupply.ChargeThresholdLow);
}
}  // namespace seb::configuration::configuration_data::data_mapping
