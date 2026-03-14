#pragma once
namespace seb::userinterface::contracts::shell { class INotificationControl { public: virtual ~INotificationControl() = default; virtual void setVisible(bool visible) = 0; }; }
