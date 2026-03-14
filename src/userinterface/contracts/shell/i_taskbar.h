#pragma once
namespace seb::userinterface::contracts::shell { class ITaskbar { public: virtual ~ITaskbar() = default; virtual void show() = 0; virtual void hide() = 0; }; }
