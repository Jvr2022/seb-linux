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
    else if (key == keys::Mouse::EnableMiddleMouseButton) settings.Mouse.AllowMiddleButton = boolean(value, settings.Mouse.AllowMiddleButton);
    else if (key == keys::Mouse::EnableRightMouseButton) settings.Mouse.AllowRightButton = boolean(value, settings.Mouse.AllowRightButton);
    else if (key.startsWith(QStringLiteral("enableF")) && key.size() <= 8) {
        const int function = key.mid(7).toInt();
        if (function >= 1 && function <= 12) {
            bool *target[] = {&keyboard.AllowF1, &keyboard.AllowF2, &keyboard.AllowF3, &keyboard.AllowF4, &keyboard.AllowF5, &keyboard.AllowF6, &keyboard.AllowF7, &keyboard.AllowF8, &keyboard.AllowF9, &keyboard.AllowF10, &keyboard.AllowF11, &keyboard.AllowF12};
            *target[function - 1] = boolean(value, *target[function - 1]);
        }
    }
}
}  // namespace seb::configuration::configuration_data::data_mapping
