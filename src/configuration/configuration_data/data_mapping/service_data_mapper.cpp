#include "service_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ServiceDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &service = settings.Service;
    if (key == keys::Service::EnableChromeNotifications && isBoolean(value)) service.DisableChromeNotifications = !boolean(value);
    else if (key == keys::Service::EnableEaseOfAccessOptions && isBoolean(value)) service.DisableEaseOfAccessOptions = !boolean(value);
    else if (key == keys::Service::EnableFindPrinter && isBoolean(value)) service.DisableFindPrinter = !boolean(value);
    else if (key == keys::Service::EnableNetworkOptions && isBoolean(value)) service.DisableNetworkOptions = !boolean(value);
    else if (key == keys::Service::EnablePasswordChange && isBoolean(value)) service.DisablePasswordChange = !boolean(value);
    else if (key == keys::Service::EnablePowerOptions && isBoolean(value)) service.DisablePowerOptions = !boolean(value);
    else if (key == keys::Service::EnableRemoteConnections && isBoolean(value)) service.DisableRemoteConnections = !boolean(value);
    else if (key == keys::Service::EnableSignout && isBoolean(value)) service.DisableSignout = !boolean(value);
    else if (key == keys::Service::EnableTaskManager && isBoolean(value)) service.DisableTaskManager = !boolean(value);
    else if (key == keys::Service::EnableUserLock && isBoolean(value)) service.DisableUserLock = !boolean(value);
    else if (key == keys::Service::EnableUserSwitch && isBoolean(value)) service.DisableUserSwitch = !boolean(value);
    else if (key == keys::Service::EnableVmwareOverlay && isBoolean(value)) service.DisableVmwareOverlay = !boolean(value);
    else if (key == keys::Service::EnableWindowsUpdate && isBoolean(value)) service.DisableWindowsUpdate = !boolean(value);
    else if (key == keys::Service::IgnoreService) service.IgnoreService = boolean(value, service.IgnoreService);
    else if (key == keys::Service::Policy && isInteger(value)) {
        switch (integer(value)) {
        case 0: service.Policy = seb::settings::service::ServicePolicy::Optional; break;
        case 1: service.Policy = seb::settings::service::ServicePolicy::Warn; break;
        case 2: service.Policy = seb::settings::service::ServicePolicy::Mandatory; break;
        default: service.Policy = seb::settings::service::ServicePolicy::Mandatory; break;
        }
    } else if (key == keys::Service::SetVmwareConfiguration) service.SetVmwareConfiguration = boolean(value, service.SetVmwareConfiguration);
}
}  // namespace seb::configuration::configuration_data::data_mapping
