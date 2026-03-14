#pragma once
namespace seb::userinterface::contracts { class IProgressIndicator { public: virtual ~IProgressIndicator() = default; virtual void setBusy(bool busy) = 0; virtual void setProgress(int current, int maximum) = 0; }; }
