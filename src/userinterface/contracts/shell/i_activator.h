#pragma once
namespace seb::userinterface::contracts::shell { class IActivator { public: virtual ~IActivator() = default; virtual void setEnabled(bool enabled) = 0; }; }
