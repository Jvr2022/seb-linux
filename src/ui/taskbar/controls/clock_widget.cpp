#include "clock_widget.h"

#include "../common/taskbar_style.h"

#include <QDateTime>
#include <QLabel>
#include <QLocale>
#include <QTimer>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

ClockWidget::ClockWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(kTaskbarHeight);
    setContentsMargins(10, 0, 10, 0);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("background: transparent;"));
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 0, 5, 0);
    layout->setSpacing(0);
    layout->setStretch(0, 1);
    layout->setStretch(1, 1);

    timeLabel_ = new QLabel(this);
    timeLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    timeLabel_->setStyleSheet(QStringLiteral("font-weight:700; color:%1; background: transparent;").arg(secondaryTextColor().name()));
    layout->addWidget(timeLabel_);

    dateLabel_ = new QLabel(this);
    dateLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    dateLabel_->setStyleSheet(QStringLiteral("color:%1; background: transparent;").arg(secondaryTextColor().name()));
    layout->addWidget(dateLabel_);

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, &QTimer::timeout, this, &ClockWidget::refresh);
    timer_->start();
    refresh();
}

void ClockWidget::refresh()
{
    const QDateTime now = QDateTime::currentDateTime();
    const QLocale locale;
    timeLabel_->setText(locale.toString(now.time(), QLocale::ShortFormat));
    dateLabel_->setText(locale.toString(now.date(), QLocale::ShortFormat));
    setToolTip(QStringLiteral("%1 %2")
                   .arg(locale.toString(now.date(), QLocale::LongFormat))
                   .arg(locale.toString(now.time(), QLocale::LongFormat)));
}

}  // namespace seb::ui::taskbar
