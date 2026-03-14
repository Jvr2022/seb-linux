#pragma once
namespace seb::userinterface::contracts::shell { class IApplicationControl { public: virtual ~IApplicationControl() = default; virtual void updateState() = 0; }; }
