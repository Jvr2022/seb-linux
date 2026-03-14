#include "external_application_control.h"

#include "../common/taskbar_button.h"
#include "../common/taskbar_popup.h"
#include "../../../applications/application_window.h"
#include "../../../applications/external_application.h"

#include <QEvent>
#include <QScrollArea>
#include <QTimer>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

ExternalApplicationControl::ExternalApplicationControl(seb::applications::ExternalApplication &application, QWidget *parent)
    : QWidget(parent)
    , application_(application)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    button_ = new TaskbarButton(this);
    button_->setFixedWidth(50);
    layout->addWidget(button_);

    popup_ = new TaskbarPopup(this);
    auto *outer = new QVBoxLayout(popup_);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto *scrollArea = new QScrollArea(popup_);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(400);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(scrollArea);
    auto *container = new QWidget(scrollArea);
    popupLayout_ = new QVBoxLayout(container);
    popupLayout_->setContentsMargins(0, 0, 0, 0);
    popupLayout_->setSpacing(0);
    scrollArea->setWidget(container);

    hideTimer_ = new QTimer(this);
    hideTimer_->setSingleShot(true);
    hideTimer_->setInterval(250);

    connect(button_, &QPushButton::clicked, this, &ExternalApplicationControl::handleClicked);
    connect(&application_, &seb::applications::ExternalApplication::windowsChanged, this, &ExternalApplicationControl::updateState);
    connect(hideTimer_, &QTimer::timeout, this, &ExternalApplicationControl::hidePopupIfInactive);
    connect(popup_, &TaskbarPopup::popupHidden, this, [this] {
        button_->setHasPopupOpen(false);
    });

    button_->installEventFilter(this);
    popup_->installEventFilter(this);
    updateState();
}

bool ExternalApplicationControl::eventFilter(QObject *watched, QEvent *event)
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

void ExternalApplicationControl::handleClicked()
{
    const QList<seb::applications::ApplicationWindow *> windows = application_.typedWindows();
    if (windows.isEmpty()) {
        application_.start();
        return;
    }
    if (windows.size() == 1) {
        windows.first()->activate();
        return;
    }

    rebuildPopup();
    button_->setHasPopupOpen(true);
    popup_->showAbove(button_);
}

void ExternalApplicationControl::hidePopupIfInactive()
{
    if (!button_->underMouse() && !popup_->underMouse()) {
        popup_->hide();
    }
}

void ExternalApplicationControl::rebuildPopup()
{
    QLayoutItem *item = nullptr;
    while ((item = popupLayout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (seb::applications::ApplicationWindow *window : application_.typedWindows()) {
        auto *button = new TaskbarButton(popup_);
        button->setMinimumWidth(250);
        button->setText(window->title());
        button->setTextMode(true);
        button->setIconPath(window->iconPath());
        connect(button, &QPushButton::clicked, this, [this, window] {
            window->activate();
            popup_->hide();
        });
        popupLayout_->addWidget(button);
    }
}

void ExternalApplicationControl::showPopupIfNeeded()
{
    if (application_.typedWindows().size() <= 1) {
        return;
    }
    rebuildPopup();
    button_->setHasPopupOpen(true);
    popup_->showAbove(button_);
}

void ExternalApplicationControl::updateState()
{
    const QList<seb::applications::ApplicationWindow *> windows = application_.typedWindows();
    button_->setActiveLineVisible(!windows.isEmpty());
    button_->setToolTip(application_.tooltip());
    button_->setIconPath(application_.iconPath());
    rebuildPopup();
}

}  // namespace seb::ui::taskbar
