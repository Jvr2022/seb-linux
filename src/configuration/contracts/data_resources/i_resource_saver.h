#pragma once

#include "../save_status.h"

#include <QByteArray>
#include <QUrl>

namespace seb::configuration::contracts::data_resources {

class IResourceSaver
{
public:
    virtual ~IResourceSaver() = default;
    virtual bool canSave(const QUrl &destination) const = 0;
    virtual SaveStatus trySave(const QUrl &destination, const QByteArray &data) const = 0;
};

}  // namespace seb::configuration::contracts::data_resources
