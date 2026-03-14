#pragma once

#include "../common/taskbar_popup.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
QT_END_NAMESPACE

class BrowserWindow;

namespace seb::ui::taskbar {

class WindowListPopup : public TaskbarPopup
{
    Q_OBJECT

public:
    explicit WindowListPopup(QWidget *parent = nullptr);

    void setWindows(const QList<BrowserWindow *> &windows);

signals:
    void windowSelected(BrowserWindow *window);

private:
    QVBoxLayout *layout_ = nullptr;
};

}  // namespace seb::ui::taskbar
