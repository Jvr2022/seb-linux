#pragma once

#include <QString>

namespace seb::userinterface::desktop::viewmodels {

class RuntimeWindowViewModel
{
public:
    void setStatus(const QString &status);
    QString status() const;

private:
    QString status_;
};

}  // namespace seb::userinterface::desktop::viewmodels
