#pragma once

#include <QWidget>

#include <functional>

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
    void setServerCallbacks(
        std::function<void(const QString &, std::function<void(bool)>)> raise,
        std::function<void(std::function<void(bool)>)> lower);
    void setMessageRequired(bool required);
    void resetRaisedState();

private:
    void updateState();
    void toggleRaisedState();

    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QLineEdit *messageEdit_ = nullptr;
    TaskbarButton *actionButton_ = nullptr;
    bool pending_ = false;
    quint64 generation_ = 0;
    bool handRaised_ = false;
    bool messageRequired_ = false;
    std::function<void(const QString &, std::function<void(bool)>)> raiseCallback_;
    std::function<void(std::function<void(bool)>)> lowerCallback_;
};

}  // namespace seb::ui::taskbar
