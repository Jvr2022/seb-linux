#pragma once

#include <QColor>
#include <QString>

namespace seb::ui::taskbar {

inline constexpr int kTaskbarHeight = 40;
inline constexpr int kTaskbarButtonSize = 40;
inline constexpr int kTaskbarIconSize = 22;
inline constexpr int kPopupTopMargin = 2;

inline QColor backgroundColor()
{
    return QColor(QStringLiteral("#f0f0f0"));
}

inline QColor secondaryTextColor()
{
    return QColor(QStringLiteral("#696969"));
}

inline QColor hoverBackgroundColor()
{
    return QColor(QStringLiteral("#add8e6"));
}

inline QColor hoverBorderColor()
{
    return QColor(QStringLiteral("#1e90ff"));
}

inline QColor pressedBorderColor()
{
    return QColor(QStringLiteral("#87ceeb"));
}

inline QColor activeBarColor()
{
    return QColor(QStringLiteral("#1e90ff"));
}

inline QString popupStyleSheet()
{
    return QStringLiteral(
        "QFrame { background: lightgray; border: 1px solid gray; }"
        "QLabel { color: black; }"
        "QScrollArea { background: lightgray; border: 0; }"
        "QScrollBar:vertical { background: #d8d8d8; width: 10px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: #b0b0b0; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }");
}

}  // namespace seb::ui::taskbar
