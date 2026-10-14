#include "input_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void InputDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &keyboard = settings.Keyboard;
    if (key == keys::Keyboard::AllowAltEsc) keyboard.AllowAltEsc = boolean(value, keyboard.AllowAltEsc);
    else if (key == keys::Keyboard::AllowAltF4) keyboard.AllowAltF4 = boolean(value, keyboard.AllowAltF4);
    else if (key == keys::Keyboard::AllowAltTab) keyboard.AllowAltTab = boolean(value, keyboard.AllowAltTab);
    else if (key == keys::Keyboard::AllowCtrlC) keyboard.AllowCtrlC = boolean(value, keyboard.AllowCtrlC);
    else if (key == keys::Keyboard::AllowCtrlV) keyboard.AllowCtrlV = boolean(value, keyboard.AllowCtrlV);
    else if (key == keys::Keyboard::AllowCtrlX) keyboard.AllowCtrlX = boolean(value, keyboard.AllowCtrlX);
    else if (key == keys::Keyboard::AllowCtrlEsc) keyboard.AllowCtrlEsc = boolean(value, keyboard.AllowCtrlEsc);
    else if (key == keys::Keyboard::AllowEsc) keyboard.AllowEsc = boolean(value, keyboard.AllowEsc);
    else if (key == keys::Keyboard::AllowInjected) keyboard.AllowInjected = boolean(value, keyboard.AllowInjected);
    else if (key == keys::Keyboard::AllowPrintScreen) keyboard.AllowPrintScreen = boolean(value, keyboard.AllowPrintScreen);
    else if (key == keys::Keyboard::AllowSystemKey) keyboard.AllowSystemKey = boolean(value, keyboard.AllowSystemKey);
    else if (key == keys::Keyboard::EnableKeyCheck) keyboard.EnableKeyCheck = boolean(value, keyboard.EnableKeyCheck);
    else if (key == keys::Keyboard::EnableLayoutCheck) keyboard.EnableLayoutCheck = boolean(value, keyboard.EnableLayoutCheck);
    else if (key == keys::Mouse::AllowMiddleButton) settings.Mouse.AllowMiddleButton = boolean(value, settings.Mouse.AllowMiddleButton);
    else if (key == keys::Mouse::AllowRightButton) settings.Mouse.AllowRightButton = boolean(value, settings.Mouse.AllowRightButton);
    else if (key == keys::Mouse::EnableMovementCheck) settings.Mouse.EnableMovementCheck = boolean(value, settings.Mouse.EnableMovementCheck);
    else if (key.startsWith(QStringLiteral("enableF")) && key.size() <= 8) {
        const int function = key.mid(7).toInt();
        if (function >= 1 && function <= 12) {
            bool *target[] = {&keyboard.AllowF1, &keyboard.AllowF2, &keyboard.AllowF3, &keyboard.AllowF4, &keyboard.AllowF5, &keyboard.AllowF6, &keyboard.AllowF7, &keyboard.AllowF8, &keyboard.AllowF9, &keyboard.AllowF10, &keyboard.AllowF11, &keyboard.AllowF12};
            *target[function - 1] = boolean(value, *target[function - 1]);
        }
    }
}
}  // namespace seb::configuration::configuration_data::data_mapping
