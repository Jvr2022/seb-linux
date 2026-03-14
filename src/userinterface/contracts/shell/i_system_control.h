#pragma once
namespace seb::userinterface::contracts::shell { class ISystemControl { public: virtual ~ISystemControl() = default; virtual void setEnabled(bool enabled) = 0; }; }
