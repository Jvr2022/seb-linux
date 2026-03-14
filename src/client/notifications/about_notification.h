#pragma once

#include <QObject>

class QWidget;

namespace seb::client::notifications {

class AboutNotification : public QObject
{
    Q_OBJECT

public:
    explicit AboutNotification(QWidget *parent = nullptr);

    QString iconPath() const;
    QString tooltip() const;

public slots:
    void activate();
    void terminate();

private:
    QWidget *window_ = nullptr;
};

}  // namespace seb::client::notifications
