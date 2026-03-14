#pragma once

#include <QWidget>

namespace seb::userinterface::desktop::controls::taskbar {

class TaskbarControlBase : public QWidget
{
public:
    explicit TaskbarControlBase(QWidget *parent = nullptr);
};

}  // namespace seb::userinterface::desktop::controls::taskbar
