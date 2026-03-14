#pragma once

#include "base_window.h"

class QTextEdit;

namespace seb::userinterface::desktop::windows {

class LogWindow : public BaseWindow
{
public:
    explicit LogWindow(QWidget *parent = nullptr);
    void append(const QString &line);

private:
    QTextEdit *textEdit_ = nullptr;
};

}  // namespace seb::userinterface::desktop::windows
