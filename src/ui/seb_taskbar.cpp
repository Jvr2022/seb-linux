#include "seb_taskbar.h"

#include "taskbar/taskbar_widget.h"

#include <QHBoxLayout>

SebTaskbar::SebTaskbar(SebSession &session, const seb::SebSettings &settings, QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    taskbarWidget_ = new seb::ui::taskbar::TaskbarWidget(session, settings, this);
    connect(taskbarWidget_, &seb::ui::taskbar::TaskbarWidget::quitRequested, this, &SebTaskbar::quitRequested);
    layout->addWidget(taskbarWidget_);
}

void SebTaskbar::setCurrentUrl(const QUrl &url)
{
    Q_UNUSED(url);
}

void SebTaskbar::setWindowTitleText(const QString &title)
{
    Q_UNUSED(title);
}
