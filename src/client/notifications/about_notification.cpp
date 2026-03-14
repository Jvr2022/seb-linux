#include "about_notification.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace seb::client::notifications {

AboutNotification::AboutNotification(QWidget *parent)
    : QObject(parent)
{
}

QString AboutNotification::iconPath() const
{
    return QStringLiteral(":/assets/taskbar/about.svg");
}

QString AboutNotification::tooltip() const
{
    return QStringLiteral("About Safe Exam Browser");
}

void AboutNotification::activate()
{
    if (!window_) {
        window_ = new QWidget();
        window_->setAttribute(Qt::WA_DeleteOnClose, false);
        window_->setWindowTitle(QStringLiteral("About Safe Exam Browser"));
        auto *layout = new QVBoxLayout(window_);
        layout->addWidget(new QLabel(QStringLiteral("Safe Exam Browser"), window_));
        layout->addWidget(new QLabel(QStringLiteral("Linux client port"), window_));
        connect(window_, &QWidget::destroyed, this, [this] { window_ = nullptr; });
        window_->resize(320, 180);
    }

    window_->show();
    window_->raise();
    window_->activateWindow();
}

void AboutNotification::terminate()
{
    if (window_) {
        window_->close();
    }
}

}  // namespace seb::client::notifications
