#pragma once

#include <QFrame>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace seb::ui::taskbar {

class TaskbarPopup : public QFrame
{
    Q_OBJECT

public:
    explicit TaskbarPopup(QWidget *parent = nullptr);

    void showAbove(QWidget *anchor);

signals:
    void popupHidden();

protected:
    void hideEvent(QHideEvent *event) override;
};

}  // namespace seb::ui::taskbar
