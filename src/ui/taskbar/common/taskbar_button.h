#pragma once

#include <QIcon>
#include <QPushButton>

namespace seb::ui::taskbar {

class TaskbarButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TaskbarButton(QWidget *parent = nullptr);

    void setActiveLineVisible(bool visible);
    void setHasPopupOpen(bool open);
    void setIconPath(const QString &iconPath);
    void setTextMode(bool enabled);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool activeLineVisible_ = false;
    bool hasPopupOpen_ = false;
    bool textMode_ = false;
    QIcon icon_;
};

}  // namespace seb::ui::taskbar
