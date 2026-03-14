#include "log_window.h"

#include <QTextEdit>
#include <QVBoxLayout>

namespace seb::userinterface::desktop::windows {

LogWindow::LogWindow(QWidget *parent) : BaseWindow(parent)
{
    setWindowTitle(QStringLiteral("Log"));
    auto *layout = new QVBoxLayout(this);
    textEdit_ = new QTextEdit(this);
    textEdit_->setReadOnly(true);
    layout->addWidget(textEdit_);
}

void LogWindow::append(const QString &line)
{
    textEdit_->append(line);
}

}  // namespace seb::userinterface::desktop::windows
