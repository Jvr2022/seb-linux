#pragma once
namespace seb::userinterface::contracts::windows { class IWindow { public: virtual ~IWindow() = default; virtual void show() = 0; virtual void hide() = 0; virtual void close() = 0; }; }
