#include "window_list_popup.h"

#include "../common/taskbar_button.h"

#include "../../../browser_window.h"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

namespace seb::ui::taskbar {

WindowListPopup::WindowListPopup(QWidget *parent)
    : TaskbarPopup(parent)
{
    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMaximumHeight(400);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    outer->addWidget(scrollArea);

    auto *container = new QWidget(scrollArea);
    layout_ = new QVBoxLayout(container);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    scrollArea->setWidget(container);
}

void WindowListPopup::setWindows(const QList<BrowserWindow *> &windows)
{
    QLayoutItem *item = nullptr;
    while ((item = layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    for (BrowserWindow *window : windows) {
        auto *button = new TaskbarButton(this);
        button->setMinimumWidth(250);
        button->setText(window->taskbarTitle());
        button->setTextMode(true);
        button->setIconPath(window->taskbarIconPath());
        connect(button, &QPushButton::clicked, this, [this, window] {
            emit windowSelected(window);
            hide();
        });
        layout_->addWidget(button);
    }
}

}  // namespace seb::ui::taskbar
