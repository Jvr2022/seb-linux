#pragma once
namespace seb::userinterface::contracts::proctoring { class IProctoringControl { public: virtual ~IProctoringControl() = default; virtual void setFullScreen(bool fullScreen) = 0; }; }
