#pragma once

#include <QObject>

class QWidget;

namespace seb::client::notifications {

class LogNotification : public QObject
{
    Q_OBJECT

public:
    explicit LogNotification(QWidget *parent = nullptr);

    QString iconPath() const;
    QString tooltip() const;

public slots:
    void activate();
    void terminate();

private:
    QWidget *window_ = nullptr;
};

}  // namespace seb::client::notifications
