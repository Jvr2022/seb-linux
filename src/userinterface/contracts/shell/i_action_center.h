#pragma once
namespace seb::userinterface::contracts::shell { class IActionCenter { public: virtual ~IActionCenter() = default; virtual void show() = 0; virtual void hide() = 0; }; }
