#include "keyboard_controller.h"

#include "command_helper.h"

#include <QTimer>

namespace seb::shell::taskbar::platform {

KeyboardController::KeyboardController(QObject *parent)
    : QObject(parent)
{
    timer_ = new QTimer(this);
    timer_->setInterval(15000);
    connect(timer_, &QTimer::timeout, this, &KeyboardController::refresh);
    timer_->start();
    refresh();
}

const KeyboardLayoutState &KeyboardController::state() const
{
    return state_;
}

void KeyboardController::refresh()
{
    KeyboardLayoutState next;
    const QString output = runCommand(QStringLiteral("localectl"), {QStringLiteral("status")});
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QStringLiteral("X11 Layout:"))) {
            const QString layouts = trimmed.section(':', 1).trimmed();
            next.availableLayouts = layouts.split(',', Qt::SkipEmptyParts);
        } else if (trimmed.startsWith(QStringLiteral("X11 Variant:"))) {
            const QString variant = trimmed.section(':', 1).trimmed();
            if (!variant.isEmpty()) {
                next.currentLongName = variant;
            }
        }
    }
    if (!next.availableLayouts.isEmpty()) {
        next.currentShortName = next.availableLayouts.first().trimmed().toUpper();
        if (next.currentLongName.isEmpty()) {
            next.currentLongName = next.availableLayouts.first().trimmed();
        }
    }

    if (next.currentShortName != state_.currentShortName ||
        next.currentLongName != state_.currentLongName ||
        next.availableLayouts != state_.availableLayouts) {
        state_ = next;
        emit stateChanged();
    }
}

}  // namespace seb::shell::taskbar::platform
