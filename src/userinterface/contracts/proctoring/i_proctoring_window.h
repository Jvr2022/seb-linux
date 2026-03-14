#pragma once
namespace seb::userinterface::contracts::proctoring { class IProctoringWindow { public: virtual ~IProctoringWindow() = default; virtual void show() = 0; virtual void hide() = 0; }; }
