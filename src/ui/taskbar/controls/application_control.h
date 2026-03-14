#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QTimer;
QT_END_NAMESPACE

class SebSession;
class BrowserWindow;

namespace seb::ui::taskbar {

class TaskbarButton;
class WindowListPopup;

class ApplicationControl : public QWidget
{
    Q_OBJECT

public:
    explicit ApplicationControl(SebSession &session, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void handleClicked();
    void hidePopupIfInactive();
    void showPopupIfNeeded();
    void updateState();

    SebSession &session_;
    TaskbarButton *button_ = nullptr;
    WindowListPopup *popup_ = nullptr;
    QTimer *hideTimer_ = nullptr;
};

}  // namespace seb::ui::taskbar
