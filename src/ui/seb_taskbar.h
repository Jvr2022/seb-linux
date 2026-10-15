#pragma once

#include <QWidget>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class SebSession;

namespace seb::ui::taskbar {
class TaskbarWidget;
}

class SebTaskbar : public QWidget
{
    Q_OBJECT

public:
    explicit SebTaskbar(SebSession &session, QWidget *parent = nullptr);

    void setCurrentUrl(const QUrl &url);
    void setWindowTitleText(const QString &title);

signals:
    void quitRequested();

private:
    seb::ui::taskbar::TaskbarWidget *taskbarWidget_ = nullptr;
};
