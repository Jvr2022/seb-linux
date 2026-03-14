#pragma once

#include "taskbar_model.h"

namespace seb::shell::taskbar {

class TaskbarService
{
public:
    static TaskbarModel buildModel(const seb::SebSettings &settings, bool hasBattery);
};

}  // namespace seb::shell::taskbar
