#pragma once

#include <QString>

namespace seb::userinterface::contracts::file_system_dialog {

struct FileSystemElement
{
    QString name;
    QString path;
    bool directory = false;
};

}  // namespace seb::userinterface::contracts::file_system_dialog
