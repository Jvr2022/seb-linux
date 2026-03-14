#pragma once

#include <QString>

namespace seb::browser::events {

struct PopupRequestedEventArgs
{
    QString targetUrl;
};

}  // namespace seb::browser::events
