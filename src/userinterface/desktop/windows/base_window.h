#pragma once

#include "../../contracts/windows/i_window.h"

#include <QDialog>

namespace seb::userinterface::desktop::windows {

class BaseWindow : public QDialog, public contracts::windows::IWindow
{
public:
    explicit BaseWindow(QWidget *parent = nullptr);
    void show() override;
    void hide() override;
    void close() override;
};

}  // namespace seb::userinterface::desktop::windows
