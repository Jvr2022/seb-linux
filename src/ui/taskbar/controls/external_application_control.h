#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QTimer;
class QVBoxLayout;
QT_END_NAMESPACE

namespace seb::applications {
class ApplicationWindow;
class ExternalApplication;
}

namespace seb::ui::taskbar {

class TaskbarButton;
class TaskbarPopup;

class ExternalApplicationControl : public QWidget
{
    Q_OBJECT

public:
    explicit ExternalApplicationControl(seb::applications::ExternalApplication &application, QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void handleClicked();
    void hidePopupIfInactive();
    void rebuildPopup();
    void showPopupIfNeeded();
    void updateState();

    seb::applications::ExternalApplication &application_;
    TaskbarButton *button_ = nullptr;
    TaskbarPopup *popup_ = nullptr;
    QVBoxLayout *popupLayout_ = nullptr;
    QTimer *hideTimer_ = nullptr;
};

}  // namespace seb::ui::taskbar
