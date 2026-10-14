#pragma once
namespace seb::settings::monitoring {
struct KeyboardSettings {
    bool AllowAltEsc = false;
    bool AllowAltF4 = false;
    bool AllowAltTab = true;
    bool AllowCtrlC = true;
    bool AllowCtrlV = true;
    bool AllowCtrlX = true;
    bool AllowCtrlEsc = false;
    bool AllowEsc = true;
    bool AllowF1 = true;
    bool AllowF2 = true;
    bool AllowF3 = true;
    bool AllowF4 = true;
    bool AllowF5 = true;
    bool AllowF6 = true;
    bool AllowF7 = true;
    bool AllowF8 = true;
    bool AllowF9 = true;
    bool AllowF10 = true;
    bool AllowF11 = true;
    bool AllowF12 = true;
    bool AllowInjected = false;
    bool AllowPrintScreen = false;
    bool AllowSystemKey = false;
    bool EnableKeyCheck = false;
    bool EnableLayoutCheck = false;
};
}
