#pragma once

#include "javascript_dialog_type.h"

#include <QString>

namespace seb::browser::events {

struct JavaScriptDialogRequestedEventArgs
{
    JavaScriptDialogType type = JavaScriptDialogType::Alert;
    QString message;
    QString promptText;
};

}  // namespace seb::browser::events
