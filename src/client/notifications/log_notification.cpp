#include "log_notification.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace seb::client::notifications {

LogNotification::LogNotification(QWidget *parent)
    : QObject(parent)
{
}

QString LogNotification::iconPath() const
{
    return QStringLiteral(":/assets/taskbar/log.svg");
}

QString LogNotification::tooltip() const
{
    return QStringLiteral("SEB log");
}

void LogNotification::activate()
{
    if (!window_) {
        window_ = new QWidget();
        window_->setWindowTitle(QStringLiteral("Safe Exam Browser Log"));
        auto *layout = new QVBoxLayout(window_);
        layout->addWidget(new QLabel(QStringLiteral("Runtime log output"), window_));
        auto *text = new QPlainTextEdit(window_);
        text->setReadOnly(true);
        text->setPlainText(QStringLiteral("Log window integration is hosted by the Linux client subsystem."));
        layout->addWidget(text);
        connect(window_, &QWidget::destroyed, this, [this] { window_ = nullptr; });
        window_->resize(640, 360);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

void LogNotification::terminate()
{
    if (window_) {
        window_->close();
    }
}

}  // namespace seb::client::notifications
