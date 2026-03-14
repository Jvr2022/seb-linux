#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class RaiseHandControl : public QWidget
{
    Q_OBJECT

public:
    explicit RaiseHandControl(QWidget *parent = nullptr);

private:
    void updateState();
    void toggleRaisedState();

    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QLineEdit *messageEdit_ = nullptr;
    TaskbarButton *actionButton_ = nullptr;
    bool handRaised_ = false;
};

}  // namespace seb::ui::taskbar
