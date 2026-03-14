#include "taskbar_button.h"

#include "taskbar_style.h"

#include <QPainter>

namespace seb::ui::taskbar {

TaskbarButton::TaskbarButton(QWidget *parent)
    : QPushButton(parent)
{
    setFixedHeight(kTaskbarHeight);
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
}

void TaskbarButton::setActiveLineVisible(bool visible)
{
    activeLineVisible_ = visible;
    update();
}

void TaskbarButton::setHasPopupOpen(bool open)
{
    hasPopupOpen_ = open;
    update();
}

void TaskbarButton::setIconPath(const QString &iconPath)
{
    icon_ = QIcon(iconPath);
    update();
}

void TaskbarButton::setTextMode(bool enabled)
{
    textMode_ = enabled;
    update();
}

void TaskbarButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QColor background = backgroundColor();
    QColor border = backgroundColor();
    int borderWidth = 1;

    if (hasPopupOpen_) {
        background = QColor(Qt::lightGray);
        border = QColor(Qt::gray);
    } else if (isDown()) {
        background = hoverBackgroundColor();
        border = pressedBorderColor();
        borderWidth = 2;
    } else if (underMouse()) {
        background = hoverBackgroundColor();
        border = hoverBorderColor();
    }

    painter.fillRect(rect(), background);
    painter.setPen(QPen(border, borderWidth));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));

    if (!icon_.isNull()) {
        QRect iconRect = rect().adjusted(9, 9, -9, -9);
        if (textMode_) {
            iconRect = QRect(10, 10, 20, 20);
        }
        icon_.paint(&painter, iconRect, Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
    }

    if (textMode_) {
        painter.setPen(isEnabled() ? palette().buttonText().color() : QColor(0, 0, 0, 64));
        const int leftInset = icon_.isNull() ? 12 : 40;
        painter.drawText(rect().adjusted(leftInset, 0, -12, 0), Qt::AlignVCenter | Qt::AlignLeft, text());
    }

    if (activeLineVisible_) {
        const QRect activeRect((width() - 40) / 2, height() - 3, 40, 3);
        painter.fillRect(activeRect, activeBarColor());
    }
}

}  // namespace seb::ui::taskbar
