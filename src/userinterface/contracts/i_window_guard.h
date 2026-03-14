#pragma once
namespace seb::userinterface::contracts { class IWindowGuard { public: virtual ~IWindowGuard() = default; virtual void show() = 0; virtual void hide() = 0; }; }
