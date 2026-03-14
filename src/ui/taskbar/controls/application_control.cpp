#include "application_control.h"

#include "window_list_popup.h"
#include "../common/taskbar_button.h"
#include "../common/taskbar_style.h"

#include "../../../seb_session.h"
#include "../../../browser_window.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QTimer>

namespace seb::ui::taskbar {

ApplicationControl::ApplicationControl(SebSession &session, QWidget *parent)
    : QWidget(parent)
    , session_(session)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(50);
    button_->setToolTip(QStringLiteral("Safe Exam Browser"));
    button_->setIconPath(QStringLiteral(":/assets/icons/safe-exam-browser.png"));
    layout->addWidget(button_);

    popup_ = new WindowListPopup(this);
    hideTimer_ = new QTimer(this);
    hideTimer_->setSingleShot(true);
    hideTimer_->setInterval(250);
    connect(button_, &QPushButton::clicked, this, &ApplicationControl::handleClicked);
    connect(button_, &QPushButton::pressed, hideTimer_, qOverload<>(&QTimer::stop));
    connect(button_, &QPushButton::released, hideTimer_, qOverload<>(&QTimer::stop));
    connect(&session_, &SebSession::browserWindowsChanged, this, &ApplicationControl::updateState);
    connect(popup_, &WindowListPopup::windowSelected, &session_, &SebSession::activateWindow);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });
    connect(hideTimer_, &QTimer::timeout, this, &ApplicationControl::hidePopupIfInactive);

    button_->installEventFilter(this);
    popup_->installEventFilter(this);

    updateState();
}

bool ApplicationControl::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == button_) {
        if (event->type() == QEvent::Enter) {
            hideTimer_->stop();
            showPopupIfNeeded();
        } else if (event->type() == QEvent::Leave) {
            hideTimer_->start();
        }
    } else if (watched == popup_) {
        if (event->type() == QEvent::Enter) {
            hideTimer_->stop();
        } else if (event->type() == QEvent::Leave) {
            hideTimer_->start();
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ApplicationControl::handleClicked()
{
    const QList<BrowserWindow *> windows = session_.browserWindows();
    if (windows.isEmpty()) {
        return;
    }
    if (windows.size() == 1) {
        session_.activateWindow(windows.first());
        return;
    }

    popup_->setWindows(windows);
    button_->setHasPopupOpen(true);
    popup_->showAbove(button_);
}

void ApplicationControl::showPopupIfNeeded()
{
    const QList<BrowserWindow *> windows = session_.browserWindows();
    if (windows.size() <= 1) {
        return;
    }
    popup_->setWindows(windows);
    button_->setHasPopupOpen(true);
    popup_->showAbove(button_);
}

void ApplicationControl::hidePopupIfInactive()
{
    if (!button_->underMouse() && !popup_->underMouse()) {
        popup_->hide();
    }
}

void ApplicationControl::updateState()
{
    const QList<BrowserWindow *> windows = session_.browserWindows();
    button_->setActiveLineVisible(!windows.isEmpty());
    popup_->setWindows(windows);
}

}  // namespace seb::ui::taskbar
