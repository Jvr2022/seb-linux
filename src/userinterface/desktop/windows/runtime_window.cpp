#include "runtime_window.h"

#include <QVBoxLayout>

namespace seb::userinterface::desktop::windows {

RuntimeWindow::RuntimeWindow(QWidget *parent) : BaseWindow(parent)
{
    setWindowTitle(QStringLiteral("SEB Runtime"));
    resize(640, 360);

    auto *layout = new QVBoxLayout(this);
    statusLabel_ = new QLabel(QStringLiteral("Starting session..."), this);
    progressBar_ = new QProgressBar(this);
    logOutput_ = new QTextEdit(this);

    logOutput_->setReadOnly(true);
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);

    layout->addWidget(statusLabel_);
    layout->addWidget(progressBar_);
    layout->addWidget(logOutput_);
}

void RuntimeWindow::show()
{
    BaseWindow::show();
}

void RuntimeWindow::hide()
{
    BaseWindow::hide();
}

void RuntimeWindow::close()
{
    BaseWindow::close();
}

}  // namespace seb::userinterface::desktop::windows
