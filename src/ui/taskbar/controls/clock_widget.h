#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QTimer;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class ClockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClockWidget(QWidget *parent = nullptr);

private:
    void refresh();

    QLabel *timeLabel_ = nullptr;
    QLabel *dateLabel_ = nullptr;
    QTimer *timer_ = nullptr;
};

}  // namespace seb::ui::taskbar
