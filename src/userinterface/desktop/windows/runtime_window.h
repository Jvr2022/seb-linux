#pragma once

#include "../../contracts/windows/i_runtime_window.h"
#include "../viewmodels/runtime_window_view_model.h"
#include "base_window.h"

#include <QLabel>
#include <QProgressBar>
#include <QTextEdit>

namespace seb::userinterface::desktop::windows {

class RuntimeWindow : public BaseWindow, public contracts::windows::IRuntimeWindow
{
public:
    explicit RuntimeWindow(QWidget *parent = nullptr);

    void show() override;
    void hide() override;
    void close() override;

private:
    viewmodels::RuntimeWindowViewModel viewModel_;
    QLabel *statusLabel_ = nullptr;
    QProgressBar *progressBar_ = nullptr;
    QTextEdit *logOutput_ = nullptr;
};

}  // namespace seb::userinterface::desktop::windows
