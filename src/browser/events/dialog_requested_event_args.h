#pragma once

#include <QString>

namespace seb::browser::events {

struct DialogRequestedEventArgs
{
    QString message;
    QString title;
};

}  // namespace seb::browser::events
