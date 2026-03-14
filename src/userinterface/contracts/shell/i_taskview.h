#pragma once
namespace seb::userinterface::contracts::shell { class ITaskView { public: virtual ~ITaskView() = default; virtual void show() = 0; virtual void hide() = 0; }; }
