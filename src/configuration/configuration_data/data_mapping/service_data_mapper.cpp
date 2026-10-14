#include "service_data_mapper.h"

#include "../keys.h"
#include "mapping_helpers.h"

namespace seb::configuration::configuration_data::data_mapping {
void ServiceDataMapper::map(const QString &key, const QVariant &value, seb::settings::AppSettings &settings) const
{
    using namespace helpers;
    auto &service = settings.Service;
    if (key == keys::Service::EnableChromeNotifications) service.DisableChromeNotifications = !boolean(value, true);
    else if (key == keys::Service::EnableEaseOfAccessOptions) service.DisableEaseOfAccessOptions = !boolean(value, true);
    else if (key == keys::Service::EnableFindPrinter) service.DisableFindPrinter = !boolean(value, true);
    else if (key == keys::Service::EnableNetworkOptions) service.DisableNetworkOptions = !boolean(value, true);
    else if (key == keys::Service::EnablePasswordChange) service.DisablePasswordChange = !boolean(value, true);
    else if (key == keys::Service::EnablePowerOptions) service.DisablePowerOptions = !boolean(value, true);
    else if (key == keys::Service::EnableRemoteConnections) service.DisableRemoteConnections = !boolean(value, true);
    else if (key == keys::Service::EnableSignout) service.DisableSignout = !boolean(value, true);
    else if (key == keys::Service::EnableTaskManager) service.DisableTaskManager = !boolean(value, true);
    else if (key == keys::Service::DisableUserLock) service.DisableUserLock = !boolean(value, true);
    else if (key == keys::Service::DisableUserSwitch) service.DisableUserSwitch = !boolean(value, true);
    else if (key == keys::Service::EnableVmwareOverlay) service.DisableVmwareOverlay = !boolean(value, true);
    else if (key == keys::Service::EnableWindowsUpdate) service.DisableWindowsUpdate = !boolean(value, true);
    else if (key == keys::Service::IgnoreService) service.IgnoreService = boolean(value, service.IgnoreService);
    else if (key == keys::Service::Policy) {
        switch (integer(value)) {
        case 0: service.Policy = seb::settings::service::ServicePolicy::Optional; break;
        case 1: service.Policy = seb::settings::service::ServicePolicy::Warn; break;
        case 2: service.Policy = seb::settings::service::ServicePolicy::Mandatory; break;
        default: service.Policy = seb::settings::service::ServicePolicy::Mandatory; break;
        }
    } else if (key == keys::Service::SetVmwareConfiguration) service.SetVmwareConfiguration = boolean(value, service.SetVmwareConfiguration);
}
}  // namespace seb::configuration::configuration_data::data_mapping
