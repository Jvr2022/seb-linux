#include "taskbar_popup.h"

#include "taskbar_style.h"

#include <QHideEvent>
#include <QPoint>
#include <QWidget>

namespace seb::ui::taskbar {

TaskbarPopup::TaskbarPopup(QWidget *parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint)
{
    setAttribute(Qt::WA_DeleteOnClose, false);
    setStyleSheet(popupStyleSheet());
}

void TaskbarPopup::showAbove(QWidget *anchor)
{
    if (!anchor) {
        return;
    }

    adjustSize();
    const QPoint global = anchor->mapToGlobal(QPoint((anchor->width() - width()) / 2, -height() - kPopupTopMargin));
    move(global);
    show();
    raise();
}

void TaskbarPopup::hideEvent(QHideEvent *event)
{
    QFrame::hideEvent(event);
    emit popupHidden();
}

}  // namespace seb::ui::taskbar
