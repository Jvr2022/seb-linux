#pragma once
namespace seb::settings::systemcomponents {
struct PowerSupplySettings {
    double ChargeThresholdCritical = 0.1;
    double ChargeThresholdLow = 0.2;
};
}
