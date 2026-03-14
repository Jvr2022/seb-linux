#pragma once
#include "../../contracts/windows/i_verificator_overlay.h"
#include "base_window.h"

namespace seb::userinterface::desktop::windows {

class VerificatorOverlay : public BaseWindow, public contracts::windows::IVerificatorOverlay
{
public:
    explicit VerificatorOverlay(QWidget *parent = nullptr);

    void show() override;
    void hide() override;
    void close() override;
};

}  // namespace seb::userinterface::desktop::windows
