#include "about_window.h"

#include <QVBoxLayout>
#include <QLabel>

namespace seb::userinterface::desktop::windows {

AboutWindow::AboutWindow(QWidget *parent) : BaseWindow(parent)
{
    setWindowTitle(QStringLiteral("About Safe Exam Browser"));
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral("Safe Exam Browser"), this));
}

}  // namespace seb::userinterface::desktop::windows
