#include "input_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void InputDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &keyboard = settings.Keyboard;
    if (key == keys::Keyboard::EnableAltEsc) keyboard.AllowAltEsc = boolean(value, keyboard.AllowAltEsc);
    else if (key == keys::Keyboard::EnableAltF4) keyboard.AllowAltF4 = boolean(value, keyboard.AllowAltF4);
    else if (key == keys::Keyboard::EnableAltTab) keyboard.AllowAltTab = boolean(value, keyboard.AllowAltTab);
    else if (key == keys::Keyboard::EnableCtrlEsc) keyboard.AllowCtrlEsc = boolean(value, keyboard.AllowCtrlEsc);
    else if (key == keys::Keyboard::EnableEsc) keyboard.AllowEsc = boolean(value, keyboard.AllowEsc);
    else if (key == keys::Keyboard::EnableInjected) keyboard.AllowInjected = boolean(value, keyboard.AllowInjected);
    else if (key == keys::Keyboard::EnablePrintScreen) keyboard.AllowPrintScreen = boolean(value, keyboard.AllowPrintScreen);
    else if (key == keys::Keyboard::EnableSystemKey) keyboard.AllowSystemKey = boolean(value, keyboard.AllowSystemKey);
    else if (key == keys::Keyboard::EnableF1) keyboard.AllowF1 = boolean(value, keyboard.AllowF1);
    else if (key == keys::Keyboard::EnableF2) keyboard.AllowF2 = boolean(value, keyboard.AllowF2);
    else if (key == keys::Keyboard::EnableF3) keyboard.AllowF3 = boolean(value, keyboard.AllowF3);
    else if (key == keys::Keyboard::EnableF4) keyboard.AllowF4 = boolean(value, keyboard.AllowF4);
    else if (key == keys::Keyboard::EnableF5) keyboard.AllowF5 = boolean(value, keyboard.AllowF5);
    else if (key == keys::Keyboard::EnableF6) keyboard.AllowF6 = boolean(value, keyboard.AllowF6);
    else if (key == keys::Keyboard::EnableF7) keyboard.AllowF7 = boolean(value, keyboard.AllowF7);
    else if (key == keys::Keyboard::EnableF8) keyboard.AllowF8 = boolean(value, keyboard.AllowF8);
    else if (key == keys::Keyboard::EnableF9) keyboard.AllowF9 = boolean(value, keyboard.AllowF9);
    else if (key == keys::Keyboard::EnableF10) keyboard.AllowF10 = boolean(value, keyboard.AllowF10);
    else if (key == keys::Keyboard::EnableF11) keyboard.AllowF11 = boolean(value, keyboard.AllowF11);
    else if (key == keys::Keyboard::EnableF12) keyboard.AllowF12 = boolean(value, keyboard.AllowF12);
    else if (key == keys::Mouse::EnableMiddleMouseButton) settings.Mouse.AllowMiddleButton = boolean(value, settings.Mouse.AllowMiddleButton);
    else if (key == keys::Mouse::EnableRightMouseButton) settings.Mouse.AllowRightButton = boolean(value, settings.Mouse.AllowRightButton);
}
}  // namespace seb::configuration::configuration_data::data_mapping
